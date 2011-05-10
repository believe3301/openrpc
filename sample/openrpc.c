/*
 * openrpc.c
 *
 *  Created on: 2011-3-20
 *      Author: yanghu
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "rpc.h"

void add(rpc_conn *conn, pointer input, size_t input_len, void* data) {
	int x, y;
	x = 0;
	y = 0;
	sscanf((char*) input, "%d %d", &x, &y);
	int z = x + y;
	printf("input is %s,len is %d,x is %d,y is %d,z is %d\n", (char*) input,
			input_len, x, y, z);
	//rpc_sleep(50);//simulate delay
	rpc_return(conn, &z, sizeof(z));
}

void cb_add(rpc_conn *conn, rpc_code code, pointer output, size_t output_len,
		void* data) {
	if (code != RPC_OK) {
		fprintf(stderr, "call error %s\n", rpc_code_format(code));
		exit(1);
	}
	int result = *(int*) output;
	int i = POINTER_TO_INT(data);
	if (result != (2 * i + 1)) {
		fprintf(stderr, "i is %d,result:%d should %d\n", i, result, 2 * i + 1);
		exit(1);
	}
}

void test_big_data(rpc_conn *conn, pointer input, size_t input_len, void* data) {
	char buf[input_len];
	memcpy(buf, input, input_len);
	buf[input_len] = '\0';
	printf("get data is %s\n", buf);
	//sleep(15);
	rpc_return_null(conn);
}

//big data test
//async
//sync io

static char* test_data = "给李彦宏先生的一封信 (2011-03-26 04:33:37)转载"
	"标签： 杂谈	"
	"您好，李彦宏先生。"
	"上周我和出版社的朋友沈浩波先生去山东的纸厂销毁已经印刷完毕的一百多万册《独唱团》第二期，三百多吨的纸和工业垃圾一起进了化浆炉。"
	"几百万的损失对您来说可能是个小数目，但是对一个出版公司来说几乎等于一年白干了，那还得是国内数得上数的大出版公司。"
	"这个行业就是这么可怜的，一个一百多人的企业一年的利润还不如在上海炒一套公寓，而且分分钟要背上“黑心书商”的骂名。"
	"但是沈浩波一直很高兴，因为他说和百度的谈判终于有眉目了，百度答应派人来商量百度文库的事情，李承鹏，慕容雪村，路金波，彭浩翔，"
	"都是文化行业里数一数二的畅销书作家，导演和出版商，大家都很激动，准备了好几个晚上各种资料。"
	"于是昨天开始谈判了，您派来几个高傲的中层，始终不承认百度文库有任何的侵权行为。"
	"你们不认为那包含了几乎全中国所有最新最旧图书的279万份文档是侵权，而是网民自己上传给大家共享的。"
	"你这里只是一个平台。我觉得其实我们不用讨论平台不平台，侵权不侵权这个问题了，您其实什么都心知肚明。"
	"您在美国有那么长时间的生活经历，现在您的妻子和女儿也都在美国，您一定知道如果百度开了一个叫百度美国的搜索引擎，"
	"然后把全美国所有的作家的书和所有音乐人的音乐都放在百度美国上面免费共享会是什么样的一个结果。"
	"您不会这么做，您也不会和美国人去谈什么这只是一个平台，和我没关系，都是网民自己干的，互联网的精神是共享。"
	"因为您知道这事儿只有在现在的中国才能成立。而且您也知道谁能欺负，谁不能欺负，您看，您就没有做一个百度影剧院，让大家共享共享最新的电影电视剧。"
	"您也许不太了解出版行业，我可以简单的给您介绍一下。1999年，十二年前，我的书卖18元一本，2011年，卖25元一本，很多读者还都嫌贵。"
	"您知道这十二年间，纸张，人工，物流都涨了多少倍，但出版商一直不敢提太多价，因为怕被骂，文化人脸皮都薄。"
	"一本25元的书，一般作者的版税是百分之8，可以赚2块钱，其中还要交三毛钱左右的税，也就是可以赚一块七。"
	"一本书如果卖两万本，已经算是畅销，一个作家两年能写一本，一本可以赚三万四，一年赚一万七，如果他光写书，"
	"他得不吃不喝写一百年才够在大城市的城郊买套像样的两居室。假设一本书卖10元，里面的构成是这样的，作家赚1元，印刷成本2元多，"
	"出版社赚1元多，书店赚5元。有点名气的作家出去签售做宣传，住的都是三星的酒店，来回能坐上飞机已经算不错了。"
	"出行标准一定还不如你们的低级别员工。最近几年我已经不出席任何宣传签售活动了，但是在2004年前，"
	"我至少做过几十场各个城市的宣传活动，而在那个时候，我已经是行业里的畅销书作家，我从没住到过一次300以上的酒店，"
	"有的时候和出版社陪同的几个人得在机场等好几个小时，因为打折的那班飞机得傍晚起飞，而多住半天酒店得加钱。"
	"这个行业就是这么窘迫的。这个行业里最顶尖的企业家，年收入就几百万。出版业和互联网业，本是两个级别相当的行业，"
	"你们是用几百亿身价和私人飞机豪华游艇来算企业家身价的，我们这个行业里的企业家们，我几乎没见过一个出行坐头等舱的。"
	"我们倒不是眼红你们有钱，我们只是觉得，你们都那么富有了，为何还要一分钱都不肯花从我们这个行业里强行获得免费的知识版权。"
	"音乐人还可以靠商演赚钱，而你让作家和出版行业如何生存。也许你说，传统出版会始终消亡，但那不代表出版行业就该如此的不体面。"
	"而且文艺作品和出版行业是不会消亡的，只是换了一个介质，一开始它们被画在墙上，后来刻在竹子上，现在有书，未来也许有别的科技，"
	"但版权是永远存在的。我写这些并不是代表这个行业向你们哭穷，"
	"但这的确中国唯一一个拥有很多的资源与生活息息相关却没有什么财富可言的行业。尤其在盗版和侵权的伤害之下。"
	"我们也不是要求你们把百度文库关了，我们只是希望百度文库可以主动对版权进行保护，等未来数字阅读成熟以后，"
	"说不定百度文库还能成为中国作家生活保障的来源，而不是现在这样，成为行业公敌众矢之的。因为没有永远的敌人，也没有永远的利益。"
	"我在2006年还和磨铁图书的沈浩波先生打过笔仗，为了现代诗互相骂的不可开交，而现在却是朋友和合作伙伴。"
	"百度文库完全可以成为造福作家的基地，而不是埋葬作家的墓地。"
	"在我们这个行业里，我算是生活得好的。李彦宏先生，也许我们一样，虽不畏惧，但并不喜欢这些是非恩怨，我喜欢晒晒太阳玩泥巴，"
	"你喜欢晒晒太阳种种花。无论你怎么共享我的知识版权，至少咱俩还能一起晒晒太阳，毕竟我赛车还能养活自己和家庭，"
	"但对于大部分作家来说，他们理应靠着传统的出版和数字出版过着体面的生活。也许他们未必能够有自己的院子晒太阳。"
	"您的产品会把他们赶回阴暗的小屋里为了生活不停的写，而您头上的太阳也并不会因此大一些。"
	"中国那么多的写作者被迫为百度无偿的提供了无数的知识版权和流量，他们不光没有来找过百度麻烦或者要求百度分点红，"
	"甚至还要承受百度拥趸们的侮辱以及百度员工谈判时的蔑视。您现在是中国排名第一的企业家，作为企业家的表率，"
	"您必须对百度文库给出版行业带来的伤害有所表态。倘若百度文库始终不肯退一步，那我可以多走几步，也许在不远的某天，"
	"在您北京的办公室里往楼下望去，您可以看见我。"
	"  祝   您的女儿为她的父亲感到骄傲"
	" 韩寒"
	"2011年  3月26日"
	"end";

int main(int argc, char **argv) {
	rpc_server *server = rpc_server_create(8778);
	if (rpc_server_start(server)) {
		printf("start server\n");
	} else {
		printf("start server error\n");
	}

	rpc_server_regservice(server, "TestService", "add", add);
	rpc_server_regservice(server, "TestService", "test_big_data", test_big_data);
	rpc_client *client = rpc_client_new();
	if (!rpc_client_connect(client, "127.0.0.1", 8778)) {
		exit(1);
	}
	printf("connected server\n");

	pointer output;
	size_t len;
	char input[256];
	int i, result;

	rpc_code code;

	code = rpc_client_call(client, "TestService", "test_big_data", test_data,
			strlen(test_data), NULL, NULL);

	if (code != RPC_OK) {
		printf("call error %s\n", rpc_code_format(code));
	}

	for (i = 0; i < 0; i++) {
		len = sprintf(input, "%d %d", i, i + 1);
		printf("call!\n");
		code = rpc_client_call(client, "TestService", "add", input, strlen(
				input) + 1, &output, &len);
		if (code != RPC_OK) {
			printf("call error %s\n", rpc_code_format(code));
			exit(1);
		}
		result = *(int*) output;
		if (result != (2 * i + 1)) {
			printf("i is %d,result:%d should %d\n", i, result, 2 * i + 1);
			exit(1);
		}
		printf("result:%d\n", result);
	}
	int j;
	for (j = 0; j < 0; j++) {
		for (i = 0; i < 10000; i++) {
			len = sprintf(input, "%d %d", i, i + 1);
			rpc_client_call_async(client, "TestService", "add", strdup(input),
					strlen(input) + 1, cb_add, INT_TO_POINTER(i));
		}
	}

	printf("test ok\n");
	for (;;) {
		rpc_sleep(1000);
	}
	return EXIT_SUCCESS;
}
