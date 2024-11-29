#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>

#include <format>
#include <json.hpp>
#include <thread>

using json = nlohmann::json;

#define GITHUB_DATA_URL "https://raw.githubusercontent.com/aceinetx/EternalUtils/refs/heads/main/data.json"

namespace Eternal {
	json data;
	bool canUseData;
}

$on_mod(Loaded){
	// Get the data from server
	web::WebRequest req = web::WebRequest();
	auto task = req.get(GITHUB_DATA_URL);
	// Wait for it to finish
	while(!task.isFinished());

	Eternal::canUseData = false;
	if(task.getFinishedValue()->string().isOk()){
		Eternal::canUseData = true;

		// Parse the given data
		std::string result = task.getFinishedValue()->string().unwrap();
		Eternal::data = json::parse(result);
	} else {
		Notification::create("Eternal: failed to get data from github (Do you have internet connection?)", CCSprite::createWithSpriteFrameName("particle_193_001.png"), 2.0f)->show();
	}
}

class $modify(EternalProfilePage, ProfilePage){
	bool init(int accountID, bool ownProfile){
		if (!ProfilePage::init(accountID, ownProfile)){
			return false;
		}

		return true;
	}

	void getUserInfoFinished(GJUserScore* p0){
		ProfilePage::getUserInfoFinished(p0);

		cocos2d::CCNode* layer = (cocos2d::CCNode*)this->getChildren()->firstObject();
		std::string rank = std::format("Rank: {} (!)", p0->m_globalRank);

		layer->removeChildByID("rank-label"); // If we refresh the user page the label will add more and more, so we need to remove it first

		if(Eternal::canUseData){
			json demonlist_scores = Eternal::data["demonlist_scores"];
			json challangelist_scores = Eternal::data["challangelist_scores"];

			rank = std::format("Rank: {} (Not on lists)", p0->m_globalRank);

			int demonlist_points = -1;
			int challangelist_points = -1;
			int demonlist_place = -1;
			int challangelist_place = -1;

			for(int i=0; i<demonlist_scores.size(); i++){
				std::string username = demonlist_scores[i][0].template get<std::string>();
				int points = demonlist_scores[i][1].template get<int>();

				if(username == p0->m_userName){
					demonlist_points = points;
					demonlist_place = i+1;
				}
			}

			for(int i=0; i<challangelist_scores.size(); i++){
				std::string username = challangelist_scores[i][0].template get<std::string>();
				int points = challangelist_scores[i][1].template get<int>();

				if(username == p0->m_userName){
					challangelist_points = points;
					challangelist_place = i+1;
				}
			}

			if(demonlist_points > -1 || challangelist_points > -1) rank = std::format("Rank: {}", p0->m_globalRank);
			if(demonlist_points > -1){
				rank.append(std::format(" (Demonlist | Place: {}, Points: {})", demonlist_place, demonlist_points));
			}
			if(challangelist_points > -1){
				rank.append(std::format(" (Challangelist | Place: {}, Points: {})", challangelist_place, challangelist_points));
			}
		}

		// Remove the old global rank information
		layer->removeChildByID("global-rank-label");
		layer->removeChildByID("global-rank-hint");
		layer->removeChildByID("global-rank-icon");

		// Create a new label
		cocos2d::CCPoint rank_label_pos;
		rank_label_pos.x = 280;
		rank_label_pos.y = 270;

		auto label = CCLabelBMFont::create(rank.c_str(), "chatFont.fnt");
		label->setPosition(rank_label_pos);
		label->setScale(0.5);
		label->setID("rank-label");
		layer->addChild(label);
	}	
};

class $modify(EternalLevelInfoLayer, LevelInfoLayer){

	struct Fields {
		std::string level_name;
		std::string level_date;
		std::string level_creators;
		int top;
		bool is_eternal_level;
	};

	bool init(GJGameLevel* level, bool challange){
		if(!LevelInfoLayer::init(level, challange)){
			return false;
		}
		m_fields->is_eternal_level = false;

		if(Eternal::canUseData){
			json demonlist = Eternal::data["demonlist"];
			for(int i=0; i<demonlist.size(); i++){
				std::string level_name = demonlist[i][0];
				std::string level_creators = demonlist[i][1];
				std::string level_date = demonlist[i][2];

				if(level_name == level->m_levelName){
					m_fields->level_name = level_name;
					m_fields->level_creators = level_creators;
					m_fields->level_date = level_date;
					m_fields->top = i+1;
					m_fields->is_eternal_level = true;
				}
			}
			if(!m_fields->is_eternal_level){
				return true;
			}

			auto EternalTopMenu = CCMenu::create();
			EternalTopMenu->setPosition({151, 171});
			EternalTopMenu->setContentSize({64, 13});
			EternalTopMenu->setID("eternal-top-menu");

			auto label_spr = CCLabelBMFont::create(std::format("Top #{}", m_fields->top).c_str(), "chatFont.fnt");

			auto label = CCMenuItemSpriteExtra::create(label_spr, EternalTopMenu, menu_selector(EternalLevelInfoLayer::onTopLabelClicked));
			label->setPosition({33, 9});
			label->setScale(0.6);
			label->setID("top-label");
			
			EternalTopMenu->addChild(label);
			this->addChild(EternalTopMenu);
		}

		return true;
	}

	void onTopLabelClicked(CCObject *sender){
		this->getChildByIDRecursive("top-label")->setScale(0.6);
		if(m_fields->is_eternal_level){
			FLAlertLayer::create(
				m_fields->level_name.c_str(),
				std::format("Top: <cr>{}</c>\nClaimed it's top spot at: <cg>{}</c>\nCreators: <co>{}</c>", m_fields->top, m_fields->level_date, m_fields->level_creators),
				"OK"
			)->show();
		}
	}
};
