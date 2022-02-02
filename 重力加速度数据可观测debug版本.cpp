
#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <vector>
#include <iomanip>
using namespace std;
 //注释下面这句将关闭debug
 #define debug
 
#if defined(_WIN32)||defined(_WIN64)
#include <SDL.h>
#else 
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#endif

/* 屏幕宽高 */
/* 限制物体的运动空间 */
int SCREEN_WIDTH = 750, SCREEN_HEIGHT = 400;
const int  SCREEN_TICK_PER_FRAME = 1000 / 60;
	int mPause=0;

#define PAUSE mPause=1
/* 我们将渲染到的窗口 */
SDL_Window *gWindow;
/* 包含的窗口表面 */
SDL_Surface *gScreenSurface;
TTF_Font *font;


void init() {
	/* 初始化 */
	SDL_Init(SDL_INIT_VIDEO);

#if !defined(_WIN32)
	SDL_DisplayMode dm;
	/* 获取当前屏幕大小 */
	SDL_GetCurrentDisplayMode(0, &dm);
	SCREEN_WIDTH = dm.w, SCREEN_HEIGHT = dm.h;
#endif
	/* 创建窗口，包含标题，x与y坐标，高与宽，创建后的参数（显示） */
	gWindow = SDL_CreateWindow("test collision", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_SetWindowSize(gWindow, SCREEN_WIDTH, SCREEN_HEIGHT);
	/* 获取窗口表面进行绘制 */
	gScreenSurface = SDL_GetWindowSurface(gWindow);
TTF_Init();
font=TTF_OpenFont("msyh.ttf",38);
}
void close() {
	SDL_FreeSurface(gScreenSurface);
	SDL_DestroyWindow(gWindow);
	SDL_Quit();
}

typedef struct _Point {
	float x, y;
}Point;
struct GameObject;
vector<GameObject*> gameObjects;
struct  GameObject {
GameObject(){}	/*  设置恢复原状，较低的值会在碰撞时损失更多的能量 */
	const float restitution = 0.9;
	/* 设置默认宽度和高度 */
	int mWidth , mHeight ;
	float mass;
	float radius;
	float x, y;
	float vx, vy;
	int isColliding;
	float radians ;
		/* 转换为度 */
	float degrees =0;
	SDL_Rect mousePos, clickOffset;
	bool leftMouseButtonDown, SelectRect;
	Point vCollisionNorm ={0};
float impulse=0;
float speed=0;
int dstObj1=-1,dstObj2=-1;//,dstObj3=0,dstObj4=0;
int level=0;
	GameObject(float x, float y, float vx, float vy, float r, float m) 
		:x(x), y(y), vx(vx), vy(vy), radius(r), mass(m), isColliding(0),mWidth(r*2),mHeight(r*2){
		leftMouseButtonDown = SelectRect = 0;
	}
	GameObject(const GameObject&r) :x(r.x), y(r.y), vx(r.vx), vy(r.vy), radius(r.radius), mass(r.mass), isColliding(0),mWidth(r.radius*2),mHeight(r.radius*2){
	}
	GameObject& operator=(const GameObject&r) {
	if(&r!=this){
		x=r.x,y=r.y,vx=r.vx,vy=r.vy,radius=r.radius, mass=r.mass,isColliding=0,mWidth=r.radius*2,mHeight=r.radius*2,
		radians=r.radians,degrees=r.degrees;speed=r.speed,impulse=r.impulse,vCollisionNorm=r.vCollisionNorm,dstObj1=r.dstObj1,dstObj2=r.dstObj2;
	}
	return *this;
	}
	virtual ~GameObject() {
	}

	virtual SDL_Rect GetRect() {}
	virtual void draw() {};
	virtual void update(float) {};
	bool isRect(SDL_Rect a) {
		//SDL_PointInRect()
		return ((a.x >= x-mWidth/2 ) && (a.x <= (x + mWidth/2) ) && 
			(a.y >= y-mHeight/2 ) && (a.y <= (y+ mHeight/2) ));
	}
	void handleEvent(SDL_Event &e) {
		if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || SDL_MOUSEBUTTONUP)
		{
	
			mousePos = { e.motion.x,e.motion.y };
			if (e.type == SDL_MOUSEBUTTONUP)
			{
				if (leftMouseButtonDown&&e.button.button == SDL_BUTTON_LEFT)
					isColliding = leftMouseButtonDown = SelectRect = false;
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
			{
				leftMouseButtonDown = true;
				if (isRect(mousePos))
				{
					clickOffset = { mousePos.x - (int)x,mousePos.y - (int)y };
					isColliding = 2;
					for(auto &i:gameObjects)
					i->level=0;
					level=SelectRect = true;
					
				}
			}
			else if (e.type == SDL_MOUSEMOTION)
			{
				if (leftMouseButtonDown&&SelectRect)
				{
					isColliding = 3;
					x = mousePos.x- clickOffset.x;
					y = mousePos.y - clickOffset.y ;
				}
			}
		}
	}
};
void SetInfo(GameObject &t,string s,float value,int n);
struct  Square :public GameObject {
Square()=default;
	int color[4] = {   0x99b0,0xff8080,0xffff00,0xff00 };
public:
	Square(float x, float y, float vx, float vy, float r, float m) :GameObject(x, y, vx, vy, r, m) {
	}
	Square(const Square&r) :GameObject(r.x, r.y, r.vx, r.vy, r.radius, r.mass) {}
	/*布雷森漢姆直線演算法 */
	void draw_line(int x1, int y1, int x2, int y2)
	{
			if (SDL_MUSTLOCK(gScreenSurface))
		{
			if (SDL_LockSurface(gScreenSurface) < 0)
				exit(1);
		}
		uint32_t* pixels = (uint32_t*)gScreenSurface->pixels;
		int incx = 1, incy = 1;

		int dx = x2 - x1;
		int dy = y2 - y1;

		if (dx < 0)
			dx = -dx;
		if (dy < 0)
			dy = -dy;

		if (x2 < x1)
			incx = -1;
		if (y2 < y1)
			incy = -1;

		int x = x1;
		int y = y1;

		if (dx > dy)
		{
			// Slope less than 1
			if (!(x + y * SCREEN_WIDTH < 0 || x + y * SCREEN_WIDTH >= SCREEN_WIDTH * SCREEN_HEIGHT))
				pixels[x + y * SCREEN_WIDTH] = 0;
			int e = 2 * dy - dx;
			for (int i = 0; i < dx; i++)
			{
				if (e >= 0)
				{
					y += incy;
					e += 2 * (dy - dx);
				}
				else
				{
					e += 2 * dy;
				}
				x += incx;
				if (!(x + y * SCREEN_WIDTH < 0 || x + y * SCREEN_WIDTH >= SCREEN_WIDTH * SCREEN_HEIGHT))
					pixels[x + y * SCREEN_WIDTH] = 0;
			}

		}
		else
		{
			// Slope greater than 1
			if (!(x + y * SCREEN_WIDTH < 0 || x + y * SCREEN_WIDTH >= SCREEN_WIDTH * SCREEN_HEIGHT))
				pixels[x + y * SCREEN_WIDTH] = 0;
			int e = 2 * dx - dy;
			for (int i = 0; i < dy; i++)
			{
				if (e >= 0)
				{
					x += incx;
					e += 2 * (dx - dy);;
				}
				else
					e += 2 * dx;
				y += incy;
				if (!(x + y * SCREEN_WIDTH < 0 || x + y * SCREEN_WIDTH >= SCREEN_WIDTH * SCREEN_HEIGHT))
					pixels[x + y * SCREEN_WIDTH] = 0;
			}
		}
		
		if (SDL_MUSTLOCK(gScreenSurface))
			SDL_UnlockSurface(gScreenSurface);
	}



	void drawCircle() {
		if (SDL_MUSTLOCK(gScreenSurface))
		{
			if (SDL_LockSurface(gScreenSurface) < 0)
				exit(1);
		}

		uint32_t* pixels = (uint32_t*)gScreenSurface->pixels;
#define Mother2
#if defined(Mother1) //(小的工整)


		for (int rad = radius; rad >= 0; rad--)
		{

			// stroke a circle
			for (double i = 0; i <= 3.14 * 2; i += 0.01)
			{

				int pX = x + rad * cos(i);
				int pY = y + rad * sin(i);
				if (!(pX + pY * SCREEN_WIDTH < 0 ||
					pX + pY * SCREEN_WIDTH >= SCREEN_WIDTH * SCREEN_HEIGHT))
					pixels[pX + pY * SCREEN_WIDTH] = color[isColliding];

			}

		}
#elif  defined(Mother2)//(大圆完整)
		for (int w = 0; w < radius * 2; w++)
		{
			for (int h = 0; h < radius * 2; h++)
			{
				int dx = radius - w; // horizontal offset
				int dy = radius - h; // vertical offset
				if ((dx*dx + dy * dy) <= (radius * radius))
				{
					if (!((dx + (int)x) + (dy + (int)y)*SCREEN_WIDTH < 0 ||
						(dx + (int)x) + (dy + (int)y)*SCREEN_WIDTH >= SCREEN_WIDTH * SCREEN_HEIGHT))
						pixels[(dx + (int)x) + (dy + (int)y) * SCREEN_WIDTH] = color[isColliding];
				}
			}
		}

#endif

		if (SDL_MUSTLOCK(gScreenSurface))
			SDL_UnlockSurface(gScreenSurface);
	}

	SDL_Rect GetRect()
	{
		return { (int)x,(int)y,mWidth,mHeight };
	}
	void draw() {
		
	//	SDL_FreeSurface(load);
		/* 画一个圆 */
		drawCircle();
		/* 绘制一条直线 */
		draw_line(x, y, x + vx, y + vy);;
		
	}
	/* 重力加速度的数值约为9.81米/秒^2 */
	const float g = pow(9.81, 3);
	void update(float secondsPassed) {
	
if(isColliding>1)
return;
		/* 在地球上大约是每秒 9.81 米。您可以在游戏对象的update()函数中应用它。
		每一秒，都会在y速度上加上g，这会使物体下落的速度越来越快。 */
		vy += (g * secondsPassed);

		/* 以设定速度移动 */
		x += (vx * secondsPassed);
		y += (vy * secondsPassed);
		//cout << "x = " << x << "\t y = " << y << endl;

		/* 计算角度（vx 之前的vy） */
		radians = atan2(vy, vx);
		/* 转换为度 */
		degrees = 180 * radians / 3.14;
	}

};

float timeStamp = 0;
float secondsPassed = 0;
float oldTimeStamp = 0;
float movingSpeed = 50;
GameObject ap,bp;
int savi,savj;
void gameLoop(vector<GameObject *>&gameObjects,int mPause)
{
	/* 让我们再解释一下。当您的游戏以 60fps 运行时，大约是每帧 0.0167 秒。这意味着当您想要以每秒 50 像
		素的速度移动对象时，您必须将 50 乘以自上一帧以来经过的秒数。以 60 fps 运行的游戏将每帧移动对象
		0.835 像素。这就是update()函数中发生的事情。
		当帧率增加或减少时，移动速度也会增加。无论过去了多少时间，您的物体总是会以所需的速度移动。这
		使得动画更适合不同类型的硬件，具有不同的帧速率。当然，较低的帧率会使动画看起来不连贯，但物体
		的位移保持不变。 */
		/* 计算已经过去了多少时间 */
		if(!mPause){
	timeStamp = SDL_GetTicks();
	secondsPassed = (timeStamp - oldTimeStamp) / 1000;
	/* 第二个限制在最小0.1 */
	secondsPassed = min((double)secondsPassed, 0.1);
	//cout << secondsPassed << " " << min((double)secondsPassed, 0.1) << endl;

	oldTimeStamp = timeStamp;

	// 循环所有游戏对象
	for (size_t i = 0; i < gameObjects.size(); i++) {
		gameObjects[i]->update(secondsPassed);
	}
		}


	// 做同样的画
	for (size_t i = 0; i < gameObjects.size(); i++) {
		gameObjects[i]->draw();
	}

}

void detectEdgeCollisions(vector<GameObject *>&gameObjects)
{
	for (size_t i = 0; i < gameObjects.size(); i++)
	{
		GameObject &obj = *gameObjects[i];
		// 检查左右
		if (obj.x < obj.radius) {
			obj.vx = abs(obj.vx) * obj.restitution;
			obj.x = obj.radius;
		}
		else if (obj.x > SCREEN_WIDTH - obj.radius) {
			obj.vx = -abs(obj.vx) * obj.restitution;
			obj.x = SCREEN_WIDTH - obj.radius;
		}

		// 检查底部和顶部
		if (obj.y < obj.radius) {
			obj.vy = abs(obj.vy) * obj.restitution;
			obj.y = obj.radius;
		}
		else if (obj.y > SCREEN_HEIGHT - obj.radius) {
			obj.vy = -abs(obj.vy) * obj.restitution;
			obj.y = SCREEN_HEIGHT - obj.radius;
		}
	}
}
void showInfo1(const char *str,SDL_Color a,int x,int y,int v,int n){
		SDL_Surface*   load=TTF_RenderUTF8_Blended(font,str,a);
		
		
	SDL_Rect r={x+300*v,y+30*n,load->w/1.5,load->h/1.4};
		 SDL_Surface *opt=SDL_ConvertSurface(load,gScreenSurface->format,0);
		
	SDL_SetColorKey(opt,SDL_TRUE,SDL_MapRGB(opt->format,0,0,0));
		SDL_BlitScaled(load,0,gScreenSurface,&r);
		SDL_FreeSurface(load);
		SDL_FreeSurface(opt);
}
void SetInfo1(SDL_Color a,string s,float value,int n,int v)
{
	stringstream os;
	os<<setprecision((value-(int)value)>0?3:0)<<fixed<<s<<value;
	showInfo1(os.str().c_str(),a,0,0,n,v);
}

void SetEI(GameObject& obj,int i,int v)
{
	SetInfo1({0,0,0},u8"对象:",i+1,v,0);
		SetInfo1({0,0,255},u8"坐标x:",obj.x,v,1);
		SetInfo1({0,0,255},u8"坐标y:",obj.y,v,2);
		SetInfo1({0,0,255},u8"速度vx:",obj.vx,v,3);
		SetInfo1({0,0,255},u8"速度vy:",obj.vy,v,4);
		SetInfo1({0,0,255},u8"角度:",fmod(360+obj.degrees,360),v,5);
		SetInfo1({19,12,25},(obj.dstObj1!=-1?u8"碰撞<-:对象":u8"未发现对象 "),obj.dstObj1,v,6);
		SetInfo1({19,12,25},(obj.dstObj2!=-1?u8"碰撞->:对象":u8"未发现对象 "),obj.dstObj2,v,7);
		SetInfo1({12,22,255},u8"经过的时间:", secondsPassed,v,8);
					SetInfo1({12,22,255},u8"速度:",obj.speed,v,9);
				SetInfo1({12,22,255},u8"法向冲量:",obj.impulse,v,10);
			SetInfo1({12,22,255},u8"向量方向.x:",obj.vCollisionNorm.x,v,11);
				SetInfo1({12,22,255},u8"向量方向.y:",obj.vCollisionNorm.y,v,12);
}
bool checkCollision(SDL_Rect a, SDL_Rect b, float ar, float br) {
	float squareDistance = (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
	return squareDistance <= (ar + br)*(ar + br);
}
void detectCollisions(vector<GameObject *>&gameObjects) {

	
	// 重置所有对象的碰撞状态
	for (size_t i = 0; i < gameObjects.size(); i++)
	{
		if(gameObjects[i]->isColliding<=1)
			gameObjects[i]->isColliding = 0;
			else
			continue;
	}

	// 开始检查碰撞
	for (size_t i = 0; i < gameObjects.size(); i++)
	{
		GameObject &obj1 = *gameObjects[i];
		for (size_t j = i + 1; j < gameObjects.size(); j++)
		{
			GameObject &obj2 = *gameObjects[j];

			//比较 object1 和 object2
			if (checkCollision(obj1.GetRect(), obj2.GetRect(), obj1.radius, obj2.radius)) {
				obj1.isColliding = 1;
				obj2.isColliding = 1;
				ap=obj1,bp=obj2;
				savi=i,savj=j;
			
				/* 带有长度和方向的箭头 */
				Point vCollision = { obj2.x - obj1.x,obj2.y - obj1.y };
				/* 两点之间的距离 */
				float distance = sqrt((obj2.x - obj1.x) * (obj2.x - obj1.x) + (obj2.y - obj1.y) * (obj2.y - obj1.y));
				/* 计算归一化的碰撞向量,就得到了方向 */
				 obj1.vCollisionNorm = { vCollision.x / distance,vCollision.y / distance };
				/* 计算碰撞速度 */
				Point vRelativeVelocity = { obj1.vx - obj2.vx,obj1.vy - obj2.vy };
				obj1.speed = vRelativeVelocity.x * obj1.vCollisionNorm.x + vRelativeVelocity.y * obj1.vCollisionNorm.y;

				if (obj1.speed < 0.1)
					break;

				obj1.speed *= min(obj1.restitution, obj2.restitution);
				/* 碰撞脉冲来将质量带入方程式 */
				obj1.impulse = 2 * obj1.speed / (obj1.mass + obj2.mass);
				cout << obj1.impulse << endl;
				/*应用物理学，并通过根据速度计算碰撞脉冲来将质量带入方程。用冲量来计算动量。重的物体会把轻的推到一边。 */
				obj1.vx -= (obj1.impulse * obj2.mass * obj1.vCollisionNorm.x);
				obj1.vy -= (obj1.impulse * obj2.mass * obj1.vCollisionNorm.y);
				obj2.vx += (obj1.impulse * obj1.mass * obj1.vCollisionNorm.x);
				obj2.vy += (obj1.impulse * obj1.mass * obj1.vCollisionNorm.y);
				obj1.dstObj1=j+1;
				obj2.dstObj2=i+1;
#ifdef debug
				PAUSE;//mPause=1;
#endif
for(auto &i:gameObjects)
					i->level=0;
					obj1.level=1;
		
			}
		}
	}
}

void showInfo(const char *str,SDL_Color a,int x,int y,int n){
		SDL_Surface*   load=TTF_RenderUTF8_Blended(font,str,a);
		
		
	SDL_Rect r={x,y-380+load->h*n/1.9,load->w/1.5,load->h/1.4};
		 SDL_Surface *opt=SDL_ConvertSurface(load,gScreenSurface->format,0);
		
	SDL_SetColorKey(opt,SDL_TRUE,SDL_MapRGB(opt->format,0,0,0));
		SDL_BlitScaled(load,0,gScreenSurface,&r);
		SDL_FreeSurface(load);
		SDL_FreeSurface(opt);
}
void SetInfo(GameObject &t,SDL_Color a,string s,float value,int n){
	stringstream os;
	os<<setprecision((value-(int)value)>0?3:0)<<fixed<<s<<value;
	showInfo(os.str().c_str(),a,t.x-t.radius,t.y-t.radius,n);
}

void SetEI(GameObject &obj,int i){
	SetInfo(obj,{0,0,0},u8"对象:",i+1,0);
		SetInfo(obj,{0,0,255},u8"坐标x:",obj.x,1);
		SetInfo(obj,{0,0,255},u8"坐标y:",obj.y,2);
		SetInfo(obj,{0,0,255},u8"速度vx:",obj.vx,3);
		SetInfo(obj,{0,0,255},u8"速度vy:",obj.vy,4);
		SetInfo(obj,{0,0,255},u8"角度:",fmod(360+obj.degrees,360),5);
		SetInfo(obj,{19,12,25},(obj.dstObj1!=-1?u8"碰撞<-:对象":u8"未发现对象 "),obj.dstObj1,6);
		SetInfo(obj,{19,12,25},(obj.dstObj2!=-1?u8"碰撞->:对":u8"未发现对象 "),obj.dstObj2,7);
		SetInfo(obj,{12,22,255},u8"经过的时间:", secondsPassed,8);
					SetInfo(obj,{12,22,255},u8"速度:",obj.speed,9);
				SetInfo(obj,{12,22,255},u8"法向冲量:",obj.impulse,10);
			SetInfo(obj,{12,22,255},u8"向量方向.x:",obj.vCollisionNorm.x,11);
				SetInfo(obj,{12,22,255},u8"向量方向.y:",obj.vCollisionNorm.y,12);
}
void FillRect(int x,int y,int w,int h,SDL_Color a)
{
	SDL_Rect r={x,y,w,h};
		SDL_FillRect(gScreenSurface,&r,SDL_MapRGB(gScreenSurface->format,a.r,a.g,a.b));
}

void GameInfo(vector<GameObject*>&v)
{
	for(int i=0;i<v.size();++i){
		{
			FillRect((v[i]->x-v[i]->radius)-10,(v[i]->y-v[i]->radius)-380,380,380,{192,192,192});
			SetEI(*v[i],i);
		}
		if(mPause){
		auto it=find_if(v.begin(),
		v.end(),[&](GameObject *&p){
			return p->level;});
			if(it!=v.end())
		{
				FillRect(0,0,600,400,{12,192,192});	
				SetEI(ap,savi,0);
	SetEI(bp,savj,1);
	
				FillRect((*it)->x-(*it)->radius-10,(*it)->y-(*it)->radius-380,380,380,{12,192,10});
			
			SetEI(*(*it),(it-v.begin()));
			(*it)->draw();
		}
		}
	}
}
void StopHandle(int mPause,SDL_Rect MouseRect){
	string s;
	if(mPause)
	s="Stop";
	else
	s="Playing";

	SDL_FillRect(gScreenSurface,&MouseRect,SDL_MapRGB(gScreenSurface->format,192,192,255));
	SDL_Surface*   load=TTF_RenderUTF8_Blended(font,s.c_str(),{0,0,255});
	SDL_BlitScaled(load,0,gScreenSurface,&MouseRect);
		SDL_FreeSurface(load);
}
bool StopEvent(SDL_Event &e,SDL_Rect &MouseRect,int &mPause)
{
	if(e.type==SDL_MOUSEBUTTONDOWN)
	{
		SDL_Point MousePos={e.motion.x,e.motion.y};
		if(SDL_PointInRect(&MousePos,&MouseRect))
		{
			if(mPause==1)
			mPause=0;
			else
			mPause=1;
			return 1;
		}
	}
	return 0;
}
int main(int argc, char **argv) {
	init();
	/* 控制帧数 */
	double fpsTimer = 0, capTimer = 0;

//	vector<GameObject *> gameObjects;
	/* 坐标X,Y,速度VX,VY,半径。质量。。*/
	gameObjects.push_back(new Square(300, 50, 0, 50, 100, 100));
	gameObjects.push_back(new Square(100, 0, 0, -50, 50, 50));
	gameObjects.push_back(new Square(500, 30, 50, 50, 75, 75));
	gameObjects.push_back(new Square(500, 75, -50, 50, 65, 65));
	gameObjects.push_back(new Square(300, 300, 50, -50, 80, 80));
	
	bool quit = false;
	SDL_Event e;
	SDL_Rect MouseRect={0,0,300,200};
	MouseRect.x=SCREEN_WIDTH-MouseRect.w;

	while (!quit)
	{
		capTimer = SDL_GetTicks();

		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
				quit = true;
			for (auto& i : gameObjects)
				i->handleEvent(e);
			if(StopEvent(e,MouseRect,mPause))
			break;
		}
	//	if(mPause)
		
		/* 将窗口表面填充为白色*/
		SDL_FillRect(gScreenSurface, NULL, SDL_MapRGB(gScreenSurface->format, 255, 255, 255));
		StopHandle(mPause,MouseRect);
		gameLoop(gameObjects,mPause);
		if(!mPause){
		detectEdgeCollisions(gameObjects);
		detectCollisions(gameObjects);
		}
		GameInfo(gameObjects);
		
		/* 然后刷新它 */
		SDL_UpdateWindowSurface(gWindow);

		/* 如果每帧耗费时间小于16毫秒 */
		double frameTicks = SDL_GetTicks() - capTimer;
		if (frameTicks < SCREEN_TICK_PER_FRAME)
			//等待剩余时间
			SDL_Delay(SCREEN_TICK_PER_FRAME - frameTicks);
	}

	close();

	return 0;
}
