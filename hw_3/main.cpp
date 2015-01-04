#include <FlyWin32.h>
#include <iostream>
#include <map>
#include "local/Character.h"
#include "local/CharacterManageSystem.h"
#include "local/Camera.h"
#include "net/game_client.h"
#include "net/game_updater_real.h"
#include "src\local\Mouse.h"
#include "basic.h"
#include "src\local\MagicBall.h"
#include "src\local\CommonObjectManageSystem.h"


//#define GAME_PROGRAMMING_DEFAULT_IP "140.112.67.117"
#define GAME_PROGRAMMING_DEFAULT_IP "127.0.0.1"
#define GAME_PROGRAMMING_DEFAULT_PORT "8976"

GmClient game_client;
GmUpdaterReal game_updater;


VIEWPORTid viewportID;	//major viewe port
SCENEid sceneID;	//3d scene
OBJECTid cameraID, cameraBaseID, terrainID, lightID;
CHARACTERid actorID;
ACTIONid idleID, runID, curPoseID;
OBJECTid spID0 = FAILED_ID;
OBJECTid spID1 = FAILED_ID;

//music
AUDIOid bckMiscID, bottonMiscID;

ROOMid terrainRoomID = FAILED_ID;
TEXTid textID = FAILED_ID;
TEXTid textCharID = FAILED_ID;
TEXTid textHP_vID = FAILED_ID;
TEXTid textInfo_vID = FAILED_ID;
Camera camera;
SCENEid sID2;                // the 2D scene
SCENEid sID2menu;                // the 2D scene
GAMEFX_SYSTEMid fxSystemId;

CharacterManageSystem chrMgtSystem(&game_updater);
CommonObjectManageSystem objMgtSystem;

std::vector<GameObject*> vecCollisionTestList;

float cameraFDir[3], cameraUDir[3];


FnGameFXSystem fnFxSystem;

Mouse mouseInput;

BOOL4 DIR_KEYDOWN[4] = {FALSE, FALSE, FALSE, FALSE};
BOOL4 first_switch_action = FALSE;

char dbg_msgS[256];

//global value

int stateMenu = 0;

int frame = 0;

int wndWidth = 1024;
int wndHeight = 768;
int window_w = 1024;
int window_h = 768;
int viewPortWidth = 1024;
int viewPortHeight = 768;
int sight_w = 83;
int sight_h = 66;
int mouse_w = 20;
int mouse_h = 30;

bool bLeftButtonDown = false;

int oldX, oldY, oldXM, oldYM, oldXMM, oldYMM;
std::map<MotionState, ACTIONid> state2ActionTable;
//BOOL4 poseChange = FALSE;

//hotkey callback
void QuitGame(BYTE, BOOL4);
void Movement(BYTE, BOOL4);

//timer callback
void GameAI(int);
void RenderIt(int);

//mouse callback
void InitPivot(int, int);
void PivotCam(int, int);
void InitMove(int, int);
void MoveCam(int, int);
void InitZoom(int, int);
void ZoomCam(int, int);
void ChangeActor(BYTE code, BOOL4 value);
void setCamera();
void showMenu(BYTE code, BOOL4 value);
void updateMouseButtonStatus(int ,int);
void collisionTest();


void FyMain(int argc, char **argv)
{
	
	AllocConsole(); 
	//freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	string ip, port;
	cout << "Enter ip   [" << GAME_PROGRAMMING_DEFAULT_IP << "]: ";getline(cin, ip);
	cout << "Enter port [" << GAME_PROGRAMMING_DEFAULT_PORT << "]: ";getline(cin, port);

	if(ip.empty()) {ip = GAME_PROGRAMMING_DEFAULT_IP;}
	if(port.empty()) {port = GAME_PROGRAMMING_DEFAULT_PORT;}

	cout << "Connect to server: " << ip << "/" << port << endl;
	game_client.initialize(ip, port, &game_updater);
	game_client.connectServer();
	cout << "Server connected." << endl;

	std::cout<<"Start Game" << std::endl;
	//create a new window
	FyStartFlyWin32("HomeWork 3 - with Fly2", 0, 0, window_w, window_h, FALSE);
	
	//set up path
	FySetShaderPath("C:\\Fly2Data\\Shaders");
	FySetModelPath("C:\\Fly2Data\\Scenes");
	FySetTexturePath("C:\\Fly2Data\\Scenes\\Textures");
	FySetScenePath("C:\\Fly2Data\\Scenes");
	FySetAudioPath("C:\\Fly2Data\\Audio");
	FySetGameFXPath("C:\\Fly2Data\\GameFXFiles\\FX1");

	//create a viewport
	viewportID = FyCreateViewport(0, 0, viewPortWidth, viewPortHeight);
	FnViewport viewport(viewportID);

	mouseInput.setWindowSize(wndWidth, wndHeight);
	//create 3D scene
	sceneID = FyCreateScene(10);
	FnScene scene(sceneID);

	scene.Load("gameScene01");
	scene.SetAmbientLights(1.0f, 1.0f, 1.0f, 0.6f, 0.6f, 0.6f);

	 // create a 2D scene for sprite rendering which will be rendered on the top of 3D
    FnScene scene2D;
    sID2 = FyCreateScene(1);
    scene2D.Object(sID2);
    scene2D.SetSpriteWorldSize(1024, 768);         // 2D scene size in pixels

	FnSprite sp;
    spID0 = scene2D.CreateObject(SPRITE);
    sp.Object(spID0);
	sp.SetSize(sight_w, sight_h);
    sp.SetImage("C:\\Fly2Data\\Image\\spiner", 0, NULL, FALSE, NULL, 2, TRUE, FILTER_LINEAR);
	sp.SetPosition(window_w/2-sight_w/2, window_h/2-sight_h/2, 0);

	FnScene scene2Dmenu;
	sID2menu = FyCreateScene(1);
    scene2Dmenu.Object(sID2menu);
    scene2Dmenu.SetSpriteWorldSize(1024, 768);         // 2D scene size in pixels

	FnSprite spExit;
    OBJECTid spIDexit = scene2Dmenu.CreateObject(SPRITE);
    spExit.Object(spIDexit);
	spExit.SetSize(135, 23);
    spExit.SetImage("C:\\Fly2Data\\Image\\exit_button", 0, NULL, FALSE, NULL, 2, TRUE, FILTER_LINEAR);
	spExit.SetPosition(window_w/2-135/2, window_h/2-23/2, 0);

	FnSprite sp1;
    spID1 = scene2Dmenu.CreateObject(SPRITE);
    sp1.Object(spID1);
	sp1.SetSize(mouse_w, mouse_h);
    sp1.SetImage("C:\\Fly2Data\\Image\\mouse", 0, NULL, FALSE, NULL, 2, TRUE, FILTER_LINEAR);
	sp1.SetPosition(window_w/2-mouse_w/2, window_h/2-mouse_h/2, 0);

	//load the terrain
	terrainID = scene.CreateObject(OBJECT);
	FnObject terrain;
	terrain.ID(terrainID);
	terrain.Load("terrain");
	terrain.Show(FALSE);

	//set terrain environment
	terrainRoomID = scene.CreateRoom(SIMPLE_ROOM, 15);
	FnRoom room;
	room.ID(terrainRoomID);
	room.AddObject(terrainID);

	//load the character
	FySetModelPath("C:\\Fly2Data\\Characters");
	FySetTexturePath("C:\\Fly2Data\\Characters");
	FySetCharacterPath("C:\\Fly2Data\\Characters");

   // put the character on terrain
   float pos[3], fDir[3], uDir[3];
   pos[0] = 3569.0, pos[1] = -3108; pos[2] = 100;
	fDir[0] = 1, fDir[1] = 0; fDir[2] = 0;
	uDir[0] = 0, uDir[1] = 0, uDir[2] = 1;

	

	Character actor, ememy;
	map<int, GmCharacter*> charNode = game_client.getGmTree().getCharacterNode();
	cout << "Number of Character: " << charNode.size() << endl;
    for (map<int, GmCharacter*>::iterator it = charNode.begin(); it != charNode.end(); it++) {
		cout << it->second->game_id << ", is_main_character?" << it->second->is_main_actor << endl;
		if(it->second->is_main_actor) {
			cout << "main character" << endl;
			actor.setMeshFileName("Lyubu2");
			actor.setCharacterName("Lyubu2");
			actor.initialize(sceneID, NULL, terrainRoomID, it->second->fdir,  it->second->udir,  it->second->pos);
			actorID = actor.getCharacterId();
			chrMgtSystem.addCharacter(actor, true);
			game_updater.registerCharacter(it->second->game_id, actor.getCharacterId());
		} else {
			cout << "Not main character" << endl;
			ememy.setMeshFileName("Donzo2");
			ememy.setCharacterName("Donzo2");
			ememy.initialize(sceneID, NULL, terrainRoomID, it->second->fdir,  it->second->udir,  it->second->pos);
			chrMgtSystem.addCharacter(ememy, false);
			game_updater.registerCharacter(it->second->game_id, ememy.getCharacterId());
		}
    }
	MagicBall* magicBall = new MagicBall(actor.getCharacterId(), sceneID, terrainID, fDir, uDir, pos);
	objMgtSystem.addGameObject(magicBall);

	cameraID = scene.CreateObject(CAMERA);
	FnCamera camera;
	camera.ID(cameraID);
	camera.SetNearPlane(5.0f);
	camera.SetFarPlane(100000.0f);
	setCamera();

	//setup fx
	//fxSystemId = scene.CreateGameFXSystem();
	//fnFxSystem.ID(fxSystemId);
	//fnFxSystem.Load("Lyubu_skill04", TRUE);
	//
	//GAMEFXid allFX[1024];
	//int fxNum = fnFxSystem.GameFXNumber();
	//for (int i = 0; i < fxNum; ++i){
	//	allFX[i] = fnFxSystem.GetGameFX(i);
	//}

	//FnBaseGameFX fnGameFx(allFX[0]);
	//fnGameFx.SetPosition(pos);
	//fnGameFx.SetParentObject(actor.getCharacterId());

   // setup a point light
	
   FnLight light;
   lightID = scene.CreateObject(LIGHT);
   light.ID(lightID);
   light.Translate(70.0f, -70.0f, 70.0f, REPLACE);
   light.SetColor(1.0f, 1.0f, 1.0f);
   light.SetIntensity(1.0f);
   

   //create a text object for display message on screen
   textID = FyCreateText("Trebuchet MS", 18, FALSE, FALSE);
   textCharID = FyCreateText("Trebuchet MS", 20, TRUE, FALSE);
   textHP_vID = FyCreateText("Trebuchet MS", 60, TRUE, FALSE);
   textInfo_vID = FyCreateText("Trebuchet MS", 30, TRUE, FALSE);

   //set up audio
	bckMiscID = FyCreateAudio();
	FnAudio fnAudio(bckMiscID);
	fnAudio.Load("game_bckgnd");
	fnAudio.SetVolume(0.1);
	fnAudio.Play(LOOP);

	bottonMiscID = FyCreateAudio();
	FnAudio fnBottonAudio(bottonMiscID);
	fnBottonAudio.Load("menu_botton_on");

   // set Hotkeys
   /*
   FyDefineHotKey(FY_ESCAPE, QuitGame, FALSE);  // escape for quiting the game
   FyDefineHotKey(FY_UP, Movement, FALSE);      // Up for moving forward
   FyDefineHotKey(FY_RIGHT, Movement, FALSE);   // Right for turning right
   FyDefineHotKey(FY_LEFT, Movement, FALSE);    // Left for turning left
   FyDefineHotKey(FY_DOWN, Movement, FALSE);    // Down for moving backward
	*/
   FyDefineHotKey(FY_TAB, ChangeActor, FALSE);
   FyDefineHotKey(FY_ESC, showMenu, FALSE);
   //define some mouse function
   FyBindMouseFunction(LEFT_MOUSE, InitPivot, updateMouseButtonStatus, NULL, NULL);
   //FyBindMouseFunction(LEFT_MOUSE, InitPivot, PivotCam, NULL, NULL);
  // FyBindMouseFunction(MIDDLE_MOUSE, InitZoom, ZoomCam, NULL, NULL);
   //FyBindMouseFunction(RIGHT_MOUSE, InitMove, MoveCam, NULL, NULL);

   //bind timers, frame rate = 30 fps
   FyBindTimer(0, 30.0f, GameAI, TRUE);
   FyBindTimer(1, 30.0f, RenderIt, TRUE);

	//invoke the system
   FyInvokeFly(TRUE);

   system("pause");

   fclose(stdout);
   fclose(stderr);
   fclose(stdin);
   //FreeConsole();

}


/*-------------------------------------------------------------
  30fps timer callback in fixed frame rate for major game loop
  C.Wang 1103, 2007
 --------------------------------------------------------------*/
void GameAI(int skip)
{
	if (!stateMenu)
	{
		//collisionTest();
		mouseInput.update();
		objMgtSystem.update(skip);
		chrMgtSystem.update(skip); //人物狀態的更新
		actorID = chrMgtSystem.getActorID();
		//Camera狀態的更新
		camera.update(skip);
		camera.getCameraDir(cameraUDir, cameraFDir);
		
		game_client.update();
	}
}

void RenderIt(int skip){
	float pos[3], fDir[3], uDir[3];

	FnViewport vp;

	//render the whole scene
	vp.ID(viewportID);
	vp.Render3D(cameraID, TRUE, TRUE);
	if(stateMenu)
		vp.RenderSprites(sID2menu, FALSE, TRUE);  // no clear the background but clear the z buffer
	else
		vp.RenderSprites(sID2, FALSE, TRUE);  // no clear the background but clear the z buffer
	
	//show frame rate
	static char string[128];
	if(frame == 0){
		FyTimerReset(0);
	}

	if((frame/10)*10 == frame){
		float curTime;
		curTime = FyTimerCheckTime(0);
		sprintf_s(string, "Fps: %6.2f", frame/curTime);
	}

	frame += skip;
	if(frame >= 1000){
		frame = 0;
	}

	FnText text,charactorInfo,char_HP,info;
	text.ID(textID);
	charactorInfo.ID(textCharID);
	char_HP.ID(textHP_vID);
	info.ID(textInfo_vID);

	text.Begin(viewportID);
	charactorInfo.Begin(viewportID);
	text.Write(string, 20, 20, 255, 0, 0);

	//get camera's data
	camera.getCameraPos(pos);
	camera.getCameraDir(fDir, uDir);

	float fCameraAngle = camera.getCameraAngle();

	char posS[256], fDirS[256], uDirS[256];
	sprintf_s(posS, "pos: %8.3f %8.3f %8.3f", pos[0], pos[1], pos[2]);
	sprintf_s(fDirS, "facing: %8.3f %8.3f %8.3f", fDir[0], fDir[1], fDir[2]);
	sprintf_s(uDirS, "up: %8.3f %8.3f %8.3f", uDir[0], uDir[1], uDir[2]);

	char sCameraAngle[256], sMousePosX[256], sMousePosY[256];
	sprintf_s(sCameraAngle, "camera angle: %8.3f", fCameraAngle);
	sprintf_s(sMousePosX, "mouse X %d ", mouseInput.mousePosX);
	sprintf_s(sMousePosY, "mouse Y %d ", mouseInput.mousePosY);
	text.Write(sCameraAngle, 20, 35, 255, 255, 0);
	text.Write(sMousePosX, 20, 50, 255, 255, 0);
	text.Write(sMousePosY, 20, 65, 255, 255, 0);
    
	text.Write(posS, 20, 80, 255, 255, 0);
    text.Write(fDirS, 20, 95, 255, 255, 0);
    text.Write(uDirS, 20, 110, 255, 255, 0);

	FnCharacter actor;
	actor.ID(actorID);
	//get actor's data
	actor.GetPosition(pos);
	actor.GetDirection(fDir, uDir);
	sprintf_s(posS, "pos: %8.3f %8.3f %8.3f", pos[0], pos[1], pos[2]);
	sprintf_s(fDirS, "facing: %8.3f %8.3f %8.3f", fDir[0], fDir[1], fDir[2]);
	sprintf_s(uDirS, "up: %8.3f %8.3f %8.3f", uDir[0], uDir[1], uDir[2]);
    
	text.Write(posS, 20, 125, 255, 255, 0);
    text.Write(fDirS, 20, 140, 255, 255, 0);
    text.Write(uDirS, 20, 155, 255, 255, 0);

   text.End();

   	sprintf_s(posS, "HEALTH", chrMgtSystem.getCharacterblood(actorID));
	charactorInfo.Write(posS, 50, 700, 255, 255, 100);
	sprintf_s(posS, "ROUND", chrMgtSystem.getCharacterblood(actorID));
	charactorInfo.Write(posS, 400, 700, 255, 255, 100);
	sprintf_s(posS, "TIME", chrMgtSystem.getCharacterblood(actorID));
	charactorInfo.Write(posS, 515, 700, 255, 255, 100);
	charactorInfo.End();

	sprintf_s(posS, "%d", chrMgtSystem.getCharacterblood(actorID));
	char_HP.Write(posS, 110, 672, 255, 255, 255);
	sprintf_s(posS, "%d", chrMgtSystem.getCharacterblood(actorID));
	info.Write(posS, 460, 694 ,255, 255, 255);
	sprintf_s(posS, "%d", chrMgtSystem.getCharacterblood(actorID));
	info.Write(posS, 560, 694, 255, 255, 255);
	info.End();
   FySwapBuffers();
}



void Movement(BYTE code, BOOL4 value){

}

/*------------------
  quit the demo
  C.Wang 0327, 2005
 -------------------*/
void QuitGame(BYTE code, BOOL4 value)
{
   if (code == FY_ESCAPE) {
      if (value) {
         FyQuitFlyWin32();
      }
   }
}

/*-----------------------------------
  initialize the pivot of the camera
  C.Wang 0329, 2005
 ------------------------------------*/
void InitPivot(int x, int y)
{
   oldX = x;
   oldY = y;
   frame = 0;
}


/*------------------
  pivot the camera
  C.Wang 0329, 2005
 -------------------*/
void PivotCam(int x, int y)
{
   FnObject model;

   if (x != oldX) {
      model.ID(cameraID);
      model.Rotate(Z_AXIS, (float) 0.2*(x - oldX), GLOBAL);
      oldX = x;
   }

   if (y != oldY) {
      model.ID(cameraID);
      model.Rotate(X_AXIS, (float) 0.2*(y - oldY), GLOBAL);
      oldY = y;
   }
}

void updateMouseButtonStatus(int x, int y){
	bLeftButtonDown = true;
}


/*----------------------------------
  initialize the move of the camera
  C.Wang 0329, 2005
 -----------------------------------*/
void InitMove(int x, int y)
{
   oldXM = x;
   oldYM = y;
   frame = 0;
}


/*------------------
  move the camera
  C.Wang 0329, 2005
 -------------------*/
void MoveCam(int x, int y)
{
   if (x != oldXM) {
      FnObject model;

      model.ID(cameraID);
      model.Translate((float)(x - oldXM)*2.0f, 0.0f, 0.0f, LOCAL);
      oldXM = x;
   }
   if (y != oldYM) {
      FnObject model;

      model.ID(cameraID);
      model.Translate(0.0f, (float)(oldYM - y)*2.0f, 0.0f, LOCAL);
      oldYM = y;
   }
}


/*----------------------------------
  initialize the zoom of the camera
  C.Wang 0329, 2005
 -----------------------------------*/
void InitZoom(int x, int y)
{
   oldXMM = x;
   oldYMM = y;
   frame = 0;
}


/*------------------
  zoom the camera
  C.Wang 0329, 2005
 -------------------*/
void ZoomCam(int x, int y)
{
   if (x != oldXMM || y != oldYMM) {
      FnObject model;

      model.ID(cameraID);
      model.Translate(0.0f, 0.0f, (float)(x - oldXMM)*10.0f, LOCAL);
      oldXMM = x;
      oldYMM = y;
   }
}

void showMenu(BYTE code, BOOL4 value)
{
	if (value)
	{
		stateMenu = stateMenu ? 0 : 1;

	}
}

void ChangeActor(BYTE code, BOOL4 value)
{
	if (value)
	{
		chrMgtSystem.changActorByTAB();
		actorID = chrMgtSystem.getActorID();
		setCamera();
	}
}

void setCamera()
{
	//初始化攝影機
	camera.initialize(cameraID, terrainID, chrMgtSystem.getCameraActor());
	//放好相機
	camera.resetCamera();
}

void collisionTest(){
	float fDir[3], uDir[3], pos[3], result[3], diff[3];
	FnObject fnObject;
	for (int i = vecCollisionTestList.size()-1; i >= 0; --i){
		vecCollisionTestList[i]->getDir(fDir, uDir);
		vecCollisionTestList[i]->getPos(pos);
		fnObject.ID(vecCollisionTestList[i]->getObjectId());
		GEOMETRYid geoId = fnObject.HitTest(pos, fDir, result);
		diff[0] = result[0] - pos[0];
		diff[1] = result[1] - pos[1];
		diff[2] = result[2] - pos[2];
		float dist = diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2];
		if (dist < 10){
			vecCollisionTestList[i]->collisionEvent(geoId);
		}
		if (!vecCollisionTestList[i]->isValid){
			vecCollisionTestList.erase(vecCollisionTestList.begin() + i);
		}
	}
}

/////*==============================================================
////character movement testing using Fly2
////
////- Load a scene
////- Generate a terrain object
////- Load a character
////- Control a character to move
////- Change poses
////
////(C)2012 Chuan-Chang Wang, All Rights Reserved
////Created : 0802, 2012
////
////Last Updated : 1221, 2014, Kevin C. Wang
////===============================================================*/
////#include "FlyWin32.h"
//
//
//VIEWPORTid vID;                 // the major viewport
//SCENEid sID;                    // the 3D scene
//OBJECTid cID, tID;              // the main camera and the terrain for terrain following
////CHARACTERid actorID;            // the major character
////ACTIONid idleID, runID, curPoseID; // two actions
////ROOMid terrainRoomID = FAILED_ID;
////TEXTid textID = FAILED_ID;
//GAMEFX_SYSTEMid gFXID = FAILED_ID;
//
//// some globals
////int frame = 0;
////int oldX, oldY, oldXM, oldYM, oldXMM, oldYMM;
//
//// hotkey callbacks
//void QuitGame(BYTE, BOOL4);
//void Movement(BYTE, BOOL4);
//
//// timer callbacks
//void GameAI(int);
//void RenderIt(int);
//
//// mouse callbacks
//void InitPivot(int, int);
//void PivotCam(int, int);
//void InitMove(int, int);
//void MoveCam(int, int);
//void InitZoom(int, int);
//void ZoomCam(int, int);
//
//MagicBall mb;
//
/////*------------------
////the main program
////C.Wang 1010, 2014
////-------------------*/
//void FyMain(int argc, char **argv)
//{
//
//	// create a new world
//	BOOL4 beOK = FyStartFlyWin32("NTU@2014 Homework #01 - Use Fly2", 0, 0, 1024, 768, FALSE);
//
//	// setup the data searching paths
//	FySetShaderPath("C:\\Fly2Data\\Shaders");
//	FySetModelPath("C:\\Fly2Data\\Scenes");
//	FySetTexturePath("C:\\Fly2Data\\Scenes\\Textures");
//	FySetScenePath("C:\\Fly2Data\\Scenes");
//	FySetGameFXPath("C:\\Fly2Data\\GameFXFiles\\FX1");
//
//	// create a viewport
//	vID = FyCreateViewport(0, 0, 1024, 768);
//	FnViewport vp;
//	vp.ID(vID);
//
//	// create a 3D scene
//	sID = FyCreateScene(10);
//	FnScene scene;
//	scene.ID(sID);
//
//	// load the scene
//	scene.Load("gameScene01");
//	scene.SetAmbientLights(1.0f, 1.0f, 1.0f, 0.6f, 0.6f, 0.6f);
//
//	// load the terrain
//	tID = scene.CreateObject(OBJECT);
//	FnObject terrain;
//	terrain.ID(tID);
//	BOOL beOK1 = terrain.Load("terrain");
//	terrain.Show(FALSE);
//
//	// set terrain environment
//	terrainRoomID = scene.CreateRoom(SIMPLE_ROOM, 10);
//	FnRoom room;
//	room.ID(terrainRoomID);
//	room.AddObject(tID);
//
//	// load the character
//	FySetModelPath("C:\\Fly2Data\\Characters");
//	FySetTexturePath("C:\\Fly2Data\\Characters");
//	FySetCharacterPath("C:\\Fly2Data\\Characters");
//	actorID = scene.LoadCharacter("Lyubu2");
//
//	// put the character on terrain
//	float pos[3], fDir[3], uDir[3];
//	FnCharacter actor;
//	actor.ID(actorID);
//	pos[0] = 3569.0f; pos[1] = -3208.0f; pos[2] = 1000.0f;
//	fDir[0] = 1.0f; fDir[1] = 1.0f; fDir[2] = 0.0f;
//	uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
//	actor.SetDirection(fDir, uDir);
//
//	actor.SetTerrainRoom(terrainRoomID, 10.0f);
//	beOK = actor.PutOnTerrain(pos);
//
//	// Get two character actions pre-defined at Lyubu2
//	idleID = actor.GetBodyAction(NULL, "Idle");
//	runID = actor.GetBodyAction(NULL, "Run");
//
//	// set the character to idle action
//	curPoseID = idleID;
//	actor.SetCurrentAction(NULL, 0, curPoseID);
//	actor.Play(START, 0.0f, FALSE, TRUE);
//	actor.TurnRight(90.0f);
//	actor.Show(false, false, true, true);
//
//	// translate the camera
//	cID = scene.CreateObject(CAMERA);
//	FnCamera camera;
//	camera.ID(cID);
//	camera.SetNearPlane(5.0f);
//	camera.SetFarPlane(100000.0f);
//
//	// set camera initial position and orientation
//	pos[0] = 4315.783f; pos[1] = -3199.686f; pos[2] = 93.046f;
//	fDir[0] = -0.983f; fDir[1] = -0.143f; fDir[2] = -0.119f;
//	uDir[0] = -0.116f; uDir[1] = -0.031f; uDir[2] = 0.993f;
//	camera.SetPosition(pos);
//	camera.SetDirection(fDir, uDir);
//
//	mb.init(actorID, sID, tID, fDir, uDir, pos);
//
//	// setup a point light
//	FnLight lgt;
//	lgt.ID(scene.CreateObject(LIGHT));
//	lgt.Translate(70.0f, -70.0f, 70.0f, REPLACE);
//	lgt.SetColor(1.0f, 1.0f, 1.0f);
//	lgt.SetIntensity(1.0f);
//	lgt.SetName("MainLight");
//
//	// create a text object for displaying messages on screen
//	textID = FyCreateText("Trebuchet MS", 18, FALSE, FALSE);
//
//	// set Hotkeys
//	FyDefineHotKey(FY_ESCAPE, QuitGame, FALSE);  // escape for quiting the game
//	FyDefineHotKey(FY_UP, Movement, FALSE);      // Up for moving forward
//	FyDefineHotKey(FY_RIGHT, Movement, FALSE);   // Right for turning right
//	FyDefineHotKey(FY_LEFT, Movement, FALSE);    // Left for turning left
//
//	// define some mouse functions
//	FyBindMouseFunction(LEFT_MOUSE, InitPivot, PivotCam, NULL, NULL);
//	FyBindMouseFunction(MIDDLE_MOUSE, InitZoom, ZoomCam, NULL, NULL);
//	FyBindMouseFunction(RIGHT_MOUSE, InitMove, MoveCam, NULL, NULL);
//
//	// bind timers, frame rate = 30 fps
//	FyBindTimer(0, 30.0f, GameAI, TRUE);
//	FyBindTimer(1, 30.0f, RenderIt, TRUE);
//
//	// invoke the system
//	FyInvokeFly(TRUE);
//}
//
//
///*-------------------------------------------------------------
//30fps timer callback in fixed frame rate for major game loop
//C.Wang 1103, 2007
//--------------------------------------------------------------*/
//void GameAI(int skip)
//{
//	mb.update(skip);
//	FnCharacter actor;
//
//	// play character pose
//	actor.ID(actorID);
//	actor.Play(LOOP, (float)skip, FALSE, TRUE);
//
//	float dist = 6.0f;
//
//	// for moving forward
//	if (FyCheckHotKeyStatus(FY_UP)) {
//		actor.MoveForward(dist, TRUE, FALSE, 0.0f, TRUE);
//	}
//
//	// for turing left
//	if (FyCheckHotKeyStatus(FY_LEFT)) {
//		actor.TurnRight(-2.0f);
//	}
//
//	// for turning right
//	if (FyCheckHotKeyStatus(FY_RIGHT)) {
//		actor.TurnRight(2.0f);
//	}
//
//	// play game FX
//	if (gFXID != FAILED_ID) {
//		FnGameFXSystem gxS(gFXID);
//		BOOL4 beOK = gxS.Play((float)skip, LOOP);
//		if (!beOK) {
//			FnScene scene(sID);
//			scene.DeleteGameFXSystem(gFXID);
//			gFXID = FAILED_ID;
//		}
//	}
//}
//
//
///*----------------------
//perform the rendering
//C.Wang 0720, 2006
//-----------------------*/
//void RenderIt(int skip)
//{
//	FnViewport vp;
//
//	// render the whole scene
//	vp.ID(vID);
//	vp.Render3D(cID, TRUE, TRUE);
//
//	// get camera's data
//	FnCamera camera;
//	camera.ID(cID);
//
//	float pos[3], fDir[3], uDir[3];
//	camera.GetPosition(pos);
//	camera.GetDirection(fDir, uDir);
//
//	// show frame rate
//	static char string[128];
//	if (frame == 0) {
//		FyTimerReset(0);
//	}
//
//	if (frame / 10 * 10 == frame) {
//		float curTime;
//
//		curTime = FyTimerCheckTime(0);
//		sprintf_s(string, "Fps: %6.2f", frame / curTime);
//	}
//
//	frame += skip;
//	if (frame >= 1000) {
//		frame = 0;
//	}
//
//	FnText text;
//	text.ID(textID);
//
//	text.Begin(vID);
//	text.Write(string, 20, 20, 255, 0, 0);
//
//	char posS[256], fDirS[256], uDirS[256];
//	sprintf_s(posS, "pos: %8.3f %8.3f %8.3f", pos[0], pos[1], pos[2]);
//	sprintf_s(fDirS, "facing: %8.3f %8.3f %8.3f", fDir[0], fDir[1], fDir[2]);
//	sprintf_s(uDirS, "up: %8.3f %8.3f %8.3f", uDir[0], uDir[1], uDir[2]);
//
//	text.Write(posS, 20, 35, 255, 255, 0);
//	text.Write(fDirS, 20, 50, 255, 255, 0);
//	text.Write(uDirS, 20, 65, 255, 255, 0);
//
//	text.End();
//
//	// swap buffer
//	FySwapBuffers();
//}
//
//
///*------------------
//movement control
//C.Wang 1103, 2006
//-------------------*/
//void Movement(BYTE code, BOOL4 value)
//{
//	// Homework #01 part 2
//	FnCharacter actor;
//	actor.ID(actorID);
//
//	// if some key is released
//	if (!value) {
//		if (code == FY_UP || code == FY_LEFT || code == FY_RIGHT) {
//			// if we are running
//			if (curPoseID == runID) {
//				// and no key are pressing
//				if (!FyCheckHotKeyStatus(FY_UP) && !FyCheckHotKeyStatus(FY_LEFT) && !FyCheckHotKeyStatus(FY_RIGHT)) {
//					// change the pose to idle
//					curPoseID = idleID;
//					actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
//					actor.Play(START, 0.0f, FALSE, TRUE);
//				}
//			}
//		}
//	}
//	else {
//		// if some key is pressed
//		if (code == FY_UP || code == FY_LEFT || code == FY_RIGHT) {
//			// if the character is in idle
//			if (curPoseID == idleID) {
//				// make he to run
//				curPoseID = runID;
//				actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
//				actor.Play(START, 0.0f, FALSE, TRUE);
//			}
//		}
//	}
//}
//
//
///*------------------
//quit the demo
//C.Wang 0327, 2005
//-------------------*/
//void QuitGame(BYTE code, BOOL4 value)
//{
//	if (code == FY_ESCAPE) {
//		if (value) {
//			FyQuitFlyWin32();
//		}
//	}
//}
//
//
//
///*-----------------------------------
//initialize the pivot of the camera
//C.Wang 0329, 2005
//------------------------------------*/
//void InitPivot(int x, int y)
//{
//	oldX = x;
//	oldY = y;
//	frame = 0;
//
//	if (FyCheckHotKeyStatus(FY_SHIFT)) {
//		float xyz[3];
//
//		// hit test the terrain
//		FnViewport vp(vID);
//		GEOMETRYid gID = vp.HitPosition(tID, cID, x, y, xyz);
//		if (gID != FAILED_ID) {
//			FnScene scene(sID);
//			gFXID = scene.CreateGameFXSystem();
//
//			FnCharacter actor(actorID);
//			OBJECTid baseID = actor.GetBaseObject();
//
//			FnGameFXSystem gxS(gFXID);
//			BOOL4 beOK = gxS.Load("shockwave", TRUE);
//			gxS.SetParentObjectForAll(baseID);
//		}
//	}
//}
//
//
///*------------------
//pivot the camera
//C.Wang 0329, 2005
//-------------------*/
//void PivotCam(int x, int y)
//{
//	if (FyCheckHotKeyStatus(FY_SHIFT)) return;
//
//	FnObject model;
//
//	if (x != oldX) {
//		model.ID(cID);
//		model.Rotate(Z_AXIS, (float)(x - oldX), GLOBAL);
//		oldX = x;
//	}
//
//	if (y != oldY) {
//		model.ID(cID);
//		model.Rotate(X_AXIS, (float)(y - oldY), GLOBAL);
//		oldY = y;
//	}
//}
//
//
///*----------------------------------
//initialize the move of the camera
//C.Wang 0329, 2005
//-----------------------------------*/
//void InitMove(int x, int y)
//{
//	oldXM = x;
//	oldYM = y;
//	frame = 0;
//}
//
//
///*------------------
//move the camera
//C.Wang 0329, 2005
//-------------------*/
//void MoveCam(int x, int y)
//{
//	if (x != oldXM) {
//		FnObject model;
//
//		model.ID(cID);
//		model.Translate((float)(x - oldXM)*2.0f, 0.0f, 0.0f, LOCAL);
//		oldXM = x;
//	}
//	if (y != oldYM) {
//		FnObject model;
//
//		model.ID(cID);
//		model.Translate(0.0f, (float)(oldYM - y)*2.0f, 0.0f, LOCAL);
//		oldYM = y;
//	}
//}
//
//
///*----------------------------------
//initialize the zoom of the camera
//C.Wang 0329, 2005
//-----------------------------------*/
//void InitZoom(int x, int y)
//{
//	oldXMM = x;
//	oldYMM = y;
//	frame = 0;
//}
//
//
///*------------------
//zoom the camera
//C.Wang 0329, 2005
//-------------------*/
//void ZoomCam(int x, int y)
//{
//	if (x != oldXMM || y != oldYMM) {
//		FnObject model;
//
//		model.ID(cID);
//		model.Translate(0.0f, 0.0f, (float)(x - oldXMM)*10.0f, LOCAL);
//		oldXMM = x;
//		oldYMM = y;
//	}
//}