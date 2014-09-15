#include "idmlib/maxent_title/maxent_title.h"
#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
//cout << "hello, maxent" << endl;
//return 0;

	if (argc < 2) {
		cout << "Usage: ./maxent_title_util  model_dir" << endl;
		return 1;
	}

	idmlib::knlp::Maxent_title mt(argv[1]);

	
	// the args of the function predict is:  title, category, original_category, price, source.
	// if a doc has not the attribute of price in SCD file, please assign -1 to the fouth argument 
	string pre_cate = mt.predict("TP-LINK猫 调制解调器 电信联通宽带猫", ">电脑办公>网络设备>路由器>", ">网络设备/网络相关>ADSL MODEM/宽带猫", -69.00, "天猫");
	cout << "category: " << pre_cate << endl;
	
	pre_cate = mt.predict("儿童雨鞋韩版保暖雨靴女童可爱漂亮雨鞋水鞋水靴套鞋礼品礼物_加棉套,26码/18cm", "家居生活>居家日用>", "雨鞋>雨靴", -1, "当当网");
	cout << "category: " << pre_cate << endl;
	
	pre_cate = mt.predict("凤凰沱茶 2006年普洱 100克熟沱茶", "美食特产>茶/咖啡>茶叶>", "蔡司", -1, "当当网");
	cout << "category: " << pre_cate << endl;
}


