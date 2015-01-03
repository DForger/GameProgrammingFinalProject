#pragma once
class Mouse
{
public:
	Mouse();
	~Mouse();

	void update(){

	}

	void setMouseNewPos(int posX, int posY){
		m_nPreMousePosX = mousePosX;
		m_nPreMousePosY = mousePosY;
		mouseVelX = posX - mousePosX;
		mouseVelY = posY - mousePosY;
		mousePosX = posX;
		mousePosY = posY;
	}

public:
	int mousePosX, mousePosY;
	int mouseVelX, mouseVelY;

private:
	int m_nPreMousePosX, m_nPreMousePosY;
};

