#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/ProfilePage.hpp>

#include <format>

class $modify(EternalProfilePage, ProfilePage){
	bool init(int accountID, bool ownProfile){
		if (!ProfilePage::init(accountID, ownProfile)){
			return false;
		}



		return true;
	}

	void getUserInfoFinished(GJUserScore* p0){
		ProfilePage::getUserInfoFinished(p0);
		return;

		cocos2d::CCNode* layer = (cocos2d::CCNode*)this->getChildren()->firstObject();
		layer->removeChildByID("global-rank-label");

		cocos2d::CCPoint rank_label_pos;
		rank_label_pos.x = 160.5;
		rank_label_pos.y = 277;

		std::string rank = std::format("{} (Demonlist: )", p0->m_globalRank);

		auto label = CCLabelBMFont::create(rank.c_str(), "chatFont.fnt");
    layer->addChildAtPosition(label, Anchor::Center, rank_label_pos);
	}	
};