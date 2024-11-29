#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/ProfilePage.hpp>
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