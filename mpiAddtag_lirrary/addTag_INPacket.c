#include <linux/module.h>
#include "LSM_addVlan.h"

#include <linux/if.h>
//#include <sys/ioctl.h>
#include <linux/if_packet.h>
//#include <linux/ethernet.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <linux/ip.h> //ipヘッダ iphdr
#include <linux/tcp.h>//tcpヘッダ
#include <linux/if_ether.h>   //Etherヘッダ
#include <linux/in.h> //in_addr構x造体
//#include <stdint.h> //uint~_t

static struct packetIdentify_value stored_values;
static struct packetIdentify_value stored_valuesList[100];
static int stored_valuesList_sum = 0;

#define MYDEVICE_IOC_TYPE 'M'
#define MYDEVICE_SET_VALUES _IOW(MYDEVICE_IOC_TYPE, 1, struct packetIdentify_value)
#define MYDEVICE_GET_VALUES _IOR(MYDEVICE_IOC_TYPE, 2, struct packetIdentify_value)

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Satoru Takeuchi <satoru.takeuchi@gmail.com>");
MODULE_DESCRIPTION("Hello world kernel module");

// ioctlの登録
static struct file_operations file_ops = {
    .unlocked_ioctl = myLSM_ioctl,
    .compat_ioctl   = myLSM_ioctl,
}

// ioctl時に実行される関数
static long myLSM_ioctl(struct file *flip, unsigned int cmd, unsigned long arg){
    printk("myLSM_ioctl");
    switch(cmd) {
        case MYDEVICE_SET_VALUES:
            printk("MYDEVICE_SET_VALUES\n");
             if (!(copy_from_user(&stored_values, (void __user *)arg, sizeof(stored_values)))){
                stored_valuesList[stored_valuesList_sum] = stored_values;
                stored_valuesList_sum++;
             }else{
                return -EFAULT;
                printk(KERN_WARNING "ioctl error at myLSM_ioctl\n");
            }
            break;
        default:
            printk(KERN_WARNING "unsupported command atmyLSM_ioctl\n", cmd);
            return -EFAULT;
    }
    return 0;
}

// send(),sendTo()システムコール発行時のコールバック関数
static int myLSM_socket_sendmsg(struct socket *sock, struct msghdr *msg, int size){
    /*プロトコルヘッダ構造体の定義 msgの各ヘッダの先頭アドレスを示す*/
    char *p;                  //ヘッダの先頭を表す作業用ポインタ
    struct ether_header *eth; //Ethernetヘッダ
    struct ip *ip;            //IPヘッダ
    struct tcphder *tcp;      //TCPヘッダ
    struct udphdr *udp;       //UDPヘッダ
    p = msg;                  //msgの先頭=ehtenetヘッダの先頭
    eth = (struct ether_header *)p;
    if(ntohs(eth->ether_type) == ETHERTYPE_IP){ //ethernetのタイプがIPなら
        p += sizeof(struct ether_header);
        ip = (struct ip *)p; 
        if(ip->ip_p == IPPROTO_TCP){            //次のプロトコルがTCPなら
            p += sizeof(struct ip);
            tcp = (struct tcp *)p;
        }else return 0;
    }else return 0;

    /* MPIパケットの捜査に使用するものの定義 */
    int i = 0; //MPIパケットのリストを走査するインデックス
    int isMPIPacket = 0; //1=true
    //MPIパケットのリストのpacketIdentify_valueたち 比較元
    int dstIP; int srcIP; int dstPort; int srcPort;

    //MPIが送出したパケットかを判断する。
    for(i; i < stored_valuesList_sum; i++){
        dstIP = stored_valuesList[i]->dstIP;
        srcIP = stored_valuesList[i]->srcIP;
        dstPort = stored_valuesList[i]->dstPort;
        if(dstIP == ntons(ip->"addr") && srcIP == ntons(ip->"addr") && dstPort == ntons(tcp->"port")){
            isMPIPacket = 1;
            if(addVToP(msg, 1)){
                rmStoredFromList(i);
                size += 4;
            }
            return 0;    
        }
    }
    printf("走査終わりMPIパケットではない。");
    return 0;
}

int addVToP(struct msghdr *, int vlanType){
    //msg(受信フレームの頭)から12byte目(6byte = macアドレスのサイズ * 2 = 送受信元)から見る 
    uint16_t *p = (uint16_t *)(msg + 6 * 2);
    uint16_t ether_type = 0x8100;  //802.1Q
           
    //802.1Qタグを挿入するために4byte後ろにずらす
    //6 * 2 -> Ethernet headerのdst/src mac address分
    memmove((uint8_t *)&p[2], (uint8_t *)p, len - (6 * 2));

    //4byteずらして空けたスペースに802.1Qタグを突っ込む
    //Ethtype(前2byte)
    p[0] = htons(ether_type);
    //TCI(後ろ2bytem、後ろ12byteがVLAN ID)
    p[1] = htons(auxdata->tp_vlan_tci);

    //4byte(802.1Q tagのサイズ)伸ばす
    return 1;
}

//stored_valuesListからindex番目(0から数えて)を削除して、並び替える
int rmStoredList(int index){
    int i;
    struct packetIdentify_value tmp;
    tmp = stored_valuesList[i];
    printk("rm " + tmp);
    for(i = index; i < stored_valuesList_sum-1; i++){
        stored_valuesList[i] = stored_valuesList[i+1]        
    }
}

static void myLSMfs_unregister(void) { 
    securityfs_remove(config_ino); 
    securityfs_remove(dir_ino); 
}

static int mymodule_init(void) {
        int rc = 0;
    if((rc = myLSMfs_register())){
        printk(KERN_INFO"Failuer registering");
        return rc;
    }
    return 0;
}

static void mymodule_exit(void) {
      myLSMfs_unregister;
    printk(KERN_INFO"Root connect module removed\n");    
}

module_init(myLSM_init);
module_exit(myLSM_exit);
