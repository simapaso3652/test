// 2018_11_09 myShowPakcetの完成形
/*
 printPacketInfo()関数 追加 動作確認済み
 vlanの処理 作成中 
*/
#include "LSM_addVlan.h"    //追加

#include <stdio.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <netinet/ip.h> //ipヘッダ iphdr
#include <netinet/tcp.h>//tcpヘッダ
#include <netinet/if_ether.h>   //Etherヘッダ
#include <netinet/in.h> //in_addr構x造体
#include <stdint.h> //uint~_t

//追加
int packetLength(uint8_t *p, int max){
    
    int i, j;
    int len = 0;
    for(i = 0; i < max; i += 16) {
        for (j = 0; j < 16 && i + j < max; j++){
            len++;
            if(p[i + j] == '0') break;
        }
    }
    return len;
}

//msghdr×p_bufからp_buf=パケットを取得
void set_hdrValue(char *p_buf, struct ether_header **eth, struct ip **ip, struct tcphdr **tcp){
    char *p;                  //ヘッダの先頭を表す作業用ポインタ
    p = p_buf;                  //msgの先頭=ehtenetヘッダの先頭

    *eth = (struct ether_header *)p;
    if(ntohs((*eth)->ether_type) == ETHERTYPE_IP){ //ethernetのタイプがIPなら
        p += sizeof(struct ether_header);
        *ip = (struct ip *)p;
        if((*ip)->ip_p == IPPROTO_TCP){            //次のプロトコルがTCPなら
            p += ( (int) ((*ip)->ip_hl) << 2 );       
            *tcp = (struct tcphdr *)p;
            printf("msg is IP&TCP\n");
        }else printf("msg is IP but not TCP\n");
    }else {
        printf("msg is not IP\n");
        printf("msg's etherType->0x%4x\n", ntohs((*eth)->ether_type));
    }
}

//msghdr×p_bufからp_buf=パケットを取得
// vlanあり
void set_hdrVlanValue(char *p_buf, struct vEther_header **vEth, struct ip **ip, struct tcphdr **tcp){
    char *p;                  //ヘッダの先頭を表す作業用ポインタ
    p = p_buf;                  //msgの先頭=ehtenetヘッダの先頭
    *vEth = (struct vEther_header *)p;
    p += sizeof(struct vEther_header);
    *ip = (struct ip *)p;
    p += ( (int) ((*ip)->ip_hl) << 2 );
    *tcp = (struct tcphdr *)p;
}

void printPInfo(struct ether_header *eth, struct ip *ip, struct tcphdr *tcp){
    printf("/* paketInfo */\n");
    printf("etherType->0x%4x\n", (int)ntohs(eth->ether_type));
    char srcmac[50]; char dstmac[50];
    u_char *s = eth->ether_shost;
    u_char *d = eth->ether_dhost;
    snprintf(srcmac, 50, "%02x:%02x:%02x:%02x:%02x:%02x:", s[0],s[1],s[2],s[3],s[4],s[5]);
    snprintf(dstmac, 50, "%02x:%02x:%02x:%02x:%02x:%02x:", d[0],d[1],d[2],d[3],d[4],d[5]);
    printf("dstMAC->%s\n", dstmac);
    printf("srcMAC->%s\n", srcmac);
    printf("dstIP->%12s ", inet_ntoa(*(struct in_addr *) & (ip->ip_dst)));
    printf("srcIP->%12s, ", inet_ntoa(*(struct in_addr *) & (ip->ip_src))); //*%s(文字列)である理由
    printf("\n");
    printf("srcPort->%3u, ", ntohs(tcp->source));
    printf("dstPort->%3u", ntohs(tcp->dest));
    printf("\n/* end */\n");
}

void printVPInfo(struct vEther_header *vEth, struct ip *ip, struct tcphdr *tcp){
    printf("/* vlanPaketInfo */\n");
    printf("etherType->0x%4x\n", (int)ntohs(vEth->ether_type));
    char srcmac[50]; char dstmac[50];
    u_char *s = vEth->ether_shost;
    u_char *d = vEth->ether_dhost;
    snprintf(srcmac, 50, "%02x:%02x:%02x:%02x:%02x:%02x:", s[0],s[1],s[2],s[3],s[4],s[5]);
    snprintf(dstmac, 50, "%02x:%02x:%02x:%02x:%02x:%02x:", d[0],d[1],d[2],d[3],d[4],d[5]);
    printf("dstMAC->%s\n", dstmac);
    printf("srcMAC->%s\n", srcmac);
    printf("vlanTag->%4x\n", ntohs(vEth->vlanTag));
    printf("dstIP->%12s ", inet_ntoa(*(struct in_addr *) & (ip->ip_dst)));
    printf("srcIP->%12s, ", inet_ntoa(*(struct in_addr *) & (ip->ip_src))); //*%s(文字列)である理由
    printf("\n");
    printf("srcPort->%3u, ", ntohs(tcp->source));
    printf("dstPort->%3u", ntohs(tcp->dest));
    printf("\n/* end */\n");
}

//addVlanToPacket
int addVToP(char *p_buf, int vlanTag, int len){
    //msg(受信フレームの頭)から12byte目(6byte = macアドレスのサイズ * 2 = 送受信元)から見る 
    uint16_t *p = (uint16_t *)(p_buf + 6 * 2);
    uint16_t ether_type = 0x8100;  //802.1Q
           
    //802.1Qタグを挿入するために4byte後ろにずらす
    //6 * 2 -> Ethernet headerのdst/src mac address分
    memmove((uint8_t *)&p[2], (uint8_t *)p, len - (6 * 2));

    //4byteずらして空けたスペースに802.1Qタグを突っ込む
    //Ethtype(前2byte)
    p[0] = htons(ether_type);
    //TCI(後ろ2bytem、後ろ12byteがVLAN ID)
    //int test = 0x0c11;
    p[1] = htons(vlanTag);                

    return 1;
}

void hexdump(uint8_t *p, int count){
    int i, j;

    for(i = 0; i < count; i += 16) {
        printf("%04x : ", i);
        for (j = 0; j < 16 && i + j < count; j++)
            printf("%2.2x ", p[i + j]);
        for (; j < 16; j++) {
            printf("   ");
        }
        printf(": ");
        for (j = 0; j < 16 && i + j < count; j++) {
            char c = toascii(p[i + j]);
            printf("%c", isalnum(c) ? c : '.');
        }
        printf("\n");
    }
}

int main(void){
/*
    //追加
    struct packetIdentify_value p_ident;  //(MPI)packet identify(MPI)パケットを識別する構造体
    struct packetIdentify_value p_identList[100]; //リスト
    int sum_list = 0;           //リストの合計    

    int pd = -1;
    char ifname[] = "enp0s25";
    int ifindex;
    struct ifreq ifr;
    struct sockaddr myaddr;
    struct sockaddr_ll sll;

    uint8_t recv_buf[2048];

    struct iovec iov;
    struct msghdr msg;
    union {                 //見調べ
        struct cmsghdr cmsg;
        uint8_t buf[CMSG_SPACE(sizeof(struct tpacket_auxdata))];        //uint8_t
    } cmsgbuf;
    struct cmsghdr *cmsg;
    struct tpacket_auxdata *auxdata;

    if((pd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1){ //if((pd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1){ 
        perror("socket()");
        exit(1);
    }

    //option
    int on = 1;
    if (setsockopt(pd, SOL_PACKET, PACKET_AUXDATA, &on, sizeof(on)) == -1){
        perror("setsockopt():");
        exit(1);
    }

    //interface 
    memset(&ifr, '\0', sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(pd, SIOCGIFINDEX, &ifr) == -1) {
        perror("SIOCGIFINDEX");
        exit(1);
    }
    ifindex = ifr.ifr_ifindex;

    //HWADDR
    memset(&ifr, '\0', sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if(ioctl(pd, SIOCGIFHWADDR, &ifr) == -1){
        perror("SIOCGHIFWADDR");
        exit(1);
    }
    myaddr = ifr.ifr_hwaddr;

    memcpy(&sll, "\0", sizeof(sll));

    //bind
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_ifindex = ifindex;   

    if (bind(pd, (struct sockaddr *)&(sll), sizeof(sll)) == -1) {
        perror("bind():");
    }
    
    int len;
    int i = 0;
    int MAX = 1;
    for(i; i < MAX; i++){
        printf("it is %d\n", i);
        iov.iov_base = recv_buf;
        iov.iov_len = sizeof(recv_buf);
        memset(recv_buf, 0, sizeof(recv_buf));

        msg.msg_name = &sll;
        msg.msg_namelen = sizeof(sll);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = &cmsgbuf;
        msg.msg_controllen = sizeof(cmsgbuf);
        msg.msg_flags = 0;

        len = recvmsg(pd, &msg, MSG_TRUNC);

        if (len == -1) {
            perror("recvmsg:");
        }

        char *p_buf = (char *)(msg.msg_iov->iov_base); 
        /*プロトコルヘッダ構造体の定義 msgの各ヘッダの先頭アドレスを示す
        struct ether_header *eth; //Ethernetヘッダ
        struct ip *ip;            //IPヘッダ
        struct tcphdr *tcp;       //TCPヘッダ
        struct udphdr *udp;       //UDPヘッダ
        struct vEther_header *vEth;

        //set hdr
        set_hdrValue(p_buf, &eth, &ip, &tcp);
        printPInfo(eth, ip, tcp);

        // 受信したパケットの識別情報
        uint32_t srcIP = inet_ntoa(*(struct in_addr *) & (ip->ip_src));
        uint32_t dstIP = inet_ntoa(*(struct in_addr *) & (ip->ip_dst));
        uint16_t srcPort = ntohs(tcp->source);
        uint16_t dstPort = ntohs(tcp->dest); 
        
        // 疑似的なMPIパケットの識別リスト ここではmpiパケット=受信したパケットそのままとした。
        p_identList[sum_list].srcIP = inet_ntoa(*(struct in_addr *) & (ip->ip_src));
        p_identList[sum_list].dstIP = inet_ntoa(*(struct in_addr *) & (ip->ip_dst));
        p_identList[sum_list].srcPort = ntohs(tcp->source);
        p_identList[sum_list].dstPort = ntohs(tcp->dest);
        p_identList[sum_list].mpiType = MPI_BCAST;

        printf("len(recvmsg) : %d. myFunction's len : %d. ip_len : %d. iov_len %d\n", len,  packetLength((uint8_t *)p_buf,iov.iov_len), ip->ip_len, iov.iov_len);
        printf("p_buf\n");
        printf("vlan付与前 :hexdump\n");
        hexdump((uint8_t *)p_buf, len);

        if(p_identList[0].srcIP == srcIP && p_identList[0].dstIP == dstIP && p_identList[0].srcPort == srcPort && p_identList[0].dstPort == dstPort){
            addVToP(p_buf, p_identList[0].mpiType, 2048-4);     //test ok! //ずらせていることが確認できた。 mpiTypeが正しく挿入できていない。
            // hdr情報の更新
            set_hdrVlanValue(p_buf, &vEth, &ip, &tcp);  //追加
            printVPInfo(vEth, ip, tcp);                 //追加
            printf("vlan付与後 :hexdump\n");
            hexdump((uint8_t *)p_buf, len+4);
        }
        */
        printf("end!\n");
        printf("\n");
    //}

    return 0;
}
