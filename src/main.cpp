#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/ProfilePage.hpp>
#include <Geode/utils/web.hpp>

#include <format>
#include <json.hpp>

using json = nlohmann::json;

#define GITHUB_DATA_URL "https://raw.githubusercontent.com/aceinetx/EternalUtils/refs/heads/main/data.json"

class $modify(EternalProfilePage, ProfilePage){
	bool init(int accountID, bool ownProfile){
		if (!ProfilePage::init(accountID, ownProfile)){
			return false;
		}



		return true;
	}

	void getUserInfoFinished(GJUserScore* p0){
		ProfilePage::getUserInfoFinished(p0);

		std::string rank = std::format("{} (!)", p0->m_globalRank);

		web::WebRequest req = web::WebRequest();
		auto task = req.get(GITHUB_DATA_URL);
		while(!task.isFinished());
		if(task.getFinishedValue()->string().isOk()){
			std::string result = task.getFinishedValue()->string().unwrap();

			json data = json::parse(result);
			json demonlist_scores = data["demonlist_scores"];

			for(int i=0; i<demonlist_scores.size(); i++){
				std::string username = demonlist_scores[i][0].template get<std::string>();
				int points = demonlist_scores[i][1].template get<int>();

				if(username == p0->m_userName){
					rank = std::format("{} (Demonlist: {})", p0->m_globalRank, points);
				}
			}

		}

		cocos2d::CCNode* layer = (cocos2d::CCNode*)this->getChildren()->firstObject();
		layer->removeChildByID("global-rank-label");

		cocos2d::CCPoint rank_label_pos;
		rank_label_pos.x = 160.5;
		rank_label_pos.y = 277;


		auto label = CCLabelBMFont::create(rank.c_str(), "chatFont.fnt");
    layer->addChildAtPosition(label, Anchor::Center, rank_label_pos);
	}	
};