#include "CharacterManageSystem.h"


CharacterManageSystem::CharacterManageSystem(void)
	:m_localPlayerId(NULL)
{
}


CharacterManageSystem::~CharacterManageSystem(void)
{
}

bool CharacterManageSystem::addCharacter(Character &character, bool isLocalPlayer){
	CHARACTERid  characterId = character.getCharacterId();
	Character *pCharacter = &character;
	if (characterId == NULL){
		return true;
	}
	m_mapCharacterId2Character.insert(std::pair<CHARACTERid, Character*>(characterId, pCharacter));
	m_mapCharacterId2NewState.insert(std::pair<CHARACTERid, MotionState>(characterId, MotionState::IDLE));	
	m_mapStrName2CharacterId.insert(std::pair<std::string, CHARACTERid>(character.getCharacterName(), characterId));
	if(isLocalPlayer){	
		m_localPlayerId = characterId;
		m_FightSystem.initialize(this,&m_mapCharacterId2Character);
		character.displayMesh(false);
	}
	return false;
}

void CharacterManageSystem::updateCharacterInputs(){
	//for local player
   	int newState = 0;
	if(FyCheckHotKeyStatus(FY_W)){
		newState = newState | MotionState::MOVE_FORWARD;
		std::cout<<"up key\n";
	}												  
	if(FyCheckHotKeyStatus(FY_S)){
		newState = newState | MotionState::MOVE_BACKWARD;
		std::cout<<"down key\n";
	}
	if(FyCheckHotKeyStatus(FY_A)){
		newState = newState | MotionState::MOVE_LEFT;
		std::cout<<"left key\n";
	}
	if(FyCheckHotKeyStatus(FY_D)){
		newState = newState | MotionState::MOVE_RIGHT;
		std::cout<<"right key\n";
	}

	if (FyCheckHotKeyStatus(FY_Q)){
		newState = newState | MotionState::TURN_LEFT;
		std::cout << "left key\n";
	}
	if (FyCheckHotKeyStatus(FY_E)){
		newState = newState | MotionState::TURN_RIGHT;
		std::cout << "right key\n";
	}

	if (FyCheckHotKeyStatus(FY_F)){
		newState = newState | MotionState::ATTACK;
		std::cout<<"attak key\n";
	}
	if (mouseInput.mouseVelX > 0){
		newState = newState | MotionState::TURN_RIGHT;
	}

	if(mouseInput.mouseVelX < 0){
		newState = newState | MotionState::TURN_LEFT;
	}

	m_mapCharacterId2NewState[m_localPlayerId] = newState;

	//update other charcter's input state
	//m_mapCharacterId2NewState[m_mapStrName2CharacterId["Donzo2"]] = MotionState::IDLE;
}

void CharacterManageSystem::update(int skip){
	updateCharacterInputs();

	//update date character's animation and motion
	{
		std::map<CHARACTERid, Character*>::iterator chrIter = m_mapCharacterId2Character.begin();
		for(; chrIter != m_mapCharacterId2Character.end(); ++chrIter){
			(chrIter->second)->update(skip, m_mapCharacterId2NewState[(chrIter->first)]);
		}
	}

	//check attack
	{
//		(m_mapCharacterId2Character[m_mapStrName2CharacterId["Donzo2"] ])->modifyChrBlood(-1);	   //used to test die
		std::map<CHARACTERid, int>::iterator chrIter = m_mapCharacterId2NewState.begin();
		for(;chrIter != m_mapCharacterId2NewState.end(); ++chrIter){
			if(chrIter->second == (int)MotionState::ATTACK){
				//trigger fight system
				m_FightSystem.judgeAttack(chrIter->first);
			}
		}
	}

	//update date character's COOL_DOWN
	{
		std::map<CHARACTERid, Character*>::iterator chrIter = m_mapCharacterId2Character.begin();
		for(; chrIter != m_mapCharacterId2Character.end(); ++chrIter){
			if ((chrIter->second)->getCurrentState() == MotionState(COOL_DOWN)) //state changes from character
			{
				m_mapCharacterId2NewState[(chrIter->first)] = MotionState(COOL_DOWN);
			}
		}
	}
}

int CharacterManageSystem::getCharacterblood(CHARACTERid characterId)
{
	return m_mapCharacterId2Character[characterId]->readChrBlood();
}

CHARACTERid CharacterManageSystem::getActorID()
{
	return m_localPlayerId;
}

Character* CharacterManageSystem::getCameraActor()
{
	return m_mapCharacterId2Character[m_localPlayerId];
}

void CharacterManageSystem::gotAttacked(CHARACTERid characterId,float damage)
{
	Character* character = m_mapCharacterId2Character[characterId];
	int blood = character->modifyChrBlood(-1 * damage);
	if (blood)
	{
		m_mapCharacterId2NewState[characterId] = MotionState::DAMAGED;
	}

}

void CharacterManageSystem::changActorByTAB()
{
	m_mapCharacterId2Character[m_localPlayerId]->displayMesh(true);
	std::map<CHARACTERid, Character*>::iterator chrIter = m_mapCharacterId2Character.begin();
	for(; chrIter != m_mapCharacterId2Character.end(); ++chrIter){
		if (chrIter->first == m_localPlayerId && std::next(chrIter, 1) == m_mapCharacterId2Character.end())
		{
			chrIter->second->notOnCameraFocus();
			m_localPlayerId =  m_mapCharacterId2Character.begin()->first;
			break;
		}
		else if (chrIter->first == m_localPlayerId)
		{
			chrIter->second->notOnCameraFocus();
			m_localPlayerId = std::next(chrIter,1)->first;
			break;
		}
	}
	m_mapCharacterId2Character[m_localPlayerId]->displayMesh(false);
	return;
}