#ifndef MAXENT_TITLE_H
#define MAXENT_TITLE_H


#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <boost/lexical_cast.hpp>

#include "idmlib/maxent/maxentmodel.hpp"
#include "knlp/horse_tokenize.h"
#include "util/string/kstring.hpp"


namespace idmlib {
namespace knlp {

class Maxent_title {

public:
	Maxent_title(const std::string& model_dir);
	~Maxent_title() {if (tok_) delete tok_;};
	std::string predict(const std::string& title, const std::string& cate, 
	    const std::string& original_cate, const float& price, const std::string& source);
	
private:
	bool load_prices(const std::string& prices_file);
	bool load_cates(const std::string& cates_file);
	void seg(const std::string& title_str, std::vector<std::string>& title_words);
	std::vector<std::string> split_by_ascii(const std::string& str, const std::string tok);
private:
	maxent::MaxentModel mm_;
	ilplib::knlp::HorseTokenize *tok_;
	std::map<std::string,std::pair<float,float> > prices_;
	std::set<std::string> cates_;
	ilplib::knlp::KDictionary<const char*> ori_cate_map_;
};


Maxent_title::Maxent_title(const std::string& model_dir):ori_cate_map_(model_dir + "/ori.cate.map")
{
	std::string maxent_model_file = model_dir + "/maxent_title.model";
	mm_.load(maxent_model_file.c_str());
	
	std::string tok_dict_dir = model_dir + "/title_pca/";
	tok_ = new ilplib::knlp::HorseTokenize(tok_dict_dir);
	
	std::string price_file = model_dir + "/knn.price";
	load_prices(price_file);
	
	std::string cates_file = model_dir + "/cate.txt";
	load_cates(cates_file);
}


std::string Maxent_title::predict(const std::string& title, const std::string& cate, 
    const std::string& original_cate, const float& price, const std::string& source)
{
	std::string real_cate = cate;
	std::string real_original_cate;
	for (unsigned int i = 0; i < original_cate.size(); ++i) {
		if (original_cate[i] != ' ')
			real_original_cate += original_cate[i];
	}
	
	if (!real_cate.empty()) {
		if (real_cate[0] != '>')
			real_cate = '>' + cate;
		if (*real_cate.rbegin() == '>')
			real_cate.erase(real_cate.size() - 1);
	}
	
	if (!real_original_cate.empty()) {
		if (real_original_cate[0] != '>')
			real_original_cate = '>' + real_original_cate;
		if (*real_original_cate.rbegin() == '>')
			real_original_cate.erase(real_original_cate.size() - 1);
	}
	
	if ((real_original_cate.find("图书") != std::string::npos || 
	    real_original_cate.find("书籍") != std::string::npos || 
	    real_original_cate.find("音像") != std::string::npos) &&
	    real_original_cate.find("电子书籍") == std::string::npos) {
		return real_cate;
	}
	
	ilplib::knlp::Normalize::normalize(real_cate);
	ilplib::knlp::Normalize::normalize(real_original_cate);
		
	// filter by source
	if (source.find("淘宝") != std::string::npos || source.find("天猫") != std::string::npos) {
		std::vector<std::string> layers = split_by_ascii(real_original_cate, ">");
		for (int i = layers.size() - 1; i > 0; --i) {
			if (layers[i] == layers[i-1])
				layers.pop_back();
			else
				break;
		}
		
		real_original_cate = "";
		for (unsigned int i = 0; i < layers.size(); ++i)
			real_original_cate += '>' + layers[i];
		
		if (cates_.find(real_original_cate) != cates_.end())
			return real_original_cate;
		else
			return "";
	}
	
	// by original category map
	const char* mapped_cate_ptr = NULL;
	if (ori_cate_map_.value(real_original_cate, mapped_cate_ptr, false) == 0) {
		return std::string(mapped_cate_ptr);
	}
	
	std::vector<std::string> terms;
	std::string::size_type pos = real_original_cate.find_last_of(">");
	std::string last_layer;
	if (pos != std::string::npos)
		last_layer = real_original_cate.substr(pos + 1);
		
	std::string extended_title = last_layer + "\t" + title;
	ilplib::knlp::Normalize::normalize(extended_title);
	seg(extended_title, terms);
	std::string predicted_cate = std::string(mm_.predict(terms));
	
	// filter by price range
	if (price > 0) {
		std::map<std::string,std::pair<float,float> >::iterator ite = prices_.find(predicted_cate);
		if (ite != prices_.end()) {
			if (price < ite->second.first || price > ite->second.second) {
				predicted_cate = "";
			}
		}
	}
	
	if (predicted_cate.size() >= 7 && predicted_cate.substr(0,7) == ">手表")
		predicted_cate = ">手表";
	
	return predicted_cate;
}


void Maxent_title::seg(const std::string& title_str, std::vector<std::string>& title_words)
{
	title_words.clear();
	std::vector<std::pair<std::string, float> > tokenized_result;
	try {
		tok_->tokenize(title_str, tokenized_result);
	} catch (...) {
		return;
	}
	
	for (unsigned int i = 0; i < tokenized_result.size(); ++i) {
		if (i >= 15)
			break;
		title_words.push_back(tokenized_result[i].first);
	}
}


bool Maxent_title::load_prices(const std::string& prices_file)
{
	std::ifstream fin(prices_file.c_str());
	if (!fin) {
		std::cout << "in bool Maxent_title::load_price_ranges(const std::string&), failed to open " << prices_file << std::endl;
		return false;
	}
	std::string line;
	while (std::getline(fin,line)) {
		if (line.empty())
			continue;
		
		std::string::size_type pos1 = line.find_first_of("\t");
		std::string::size_type pos2 = line.find_first_of("\t", pos1+1);
		
		std::string cate = line.substr(0,pos1);
		float low_price = boost::lexical_cast<float>(line.substr(pos1+1, pos2 - pos1 - 1));
		float high_price = boost::lexical_cast<float>(line.substr(pos2+1));
		
		prices_[cate] = std::pair<float,float>(low_price, high_price);
	}
	fin.close();
	return true;
}


bool Maxent_title::load_cates(const std::string& cates_file)
{
	std::ifstream fin(cates_file.c_str());
	if (!fin) {
		std::cout << "in bool Maxent_title::load_cates(const std::string&), failed to open " << cates_file << std::endl;
		return false;
	}
	
	std::string line;
	while (std::getline(fin,line)) {
		if (line.empty())
			continue;
		std::string::size_type pos = line.find_first_of("\t");
		if (pos == std::string::npos)
			pos = line.size();
		cates_.insert(line.substr(0, pos));
	}
	fin.close();
	
	return true;
}


std::vector<std::string> Maxent_title::split_by_ascii(const std::string& str, const std::string tok) {
	if (str.empty())
		return std::vector<std::string>();
	if (tok.empty())
		return std::vector<std::string>(1, str);
	
	std::vector<std::string> vect;
	std::string::size_type start_pos = 0;
	
	for(unsigned int i = 0; i < str.size(); ++i) {
		if (tok.find(str[i]) != std::string::npos ) {
			unsigned int len = i - start_pos;
			if (len > 0)
				vect.push_back(str.substr(start_pos,len));
			start_pos = i + 1;
		}
	}
  	
	if(start_pos < str.size())
		vect.push_back(str.substr(start_pos));
	
	return vect;
}


} // namespace
} // namespace

#endif // MATENT_TITLE_H
