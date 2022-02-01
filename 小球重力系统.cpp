/**********重力系统*********/
/**手机c4droid可直接加载运行**/
/**改写来源js,转译c++/SDL2*****/
/****    by 桔皮沙拉    *********/
 /****qq:1598058687       ******/
 /*****包含对球的拖动********/
#include <iostream>
#include <cmath>
#include <algorithm>
using namespace std;

#if defined(_WIN32)||defined(_WIN64)
#include <SDL.h>
#else 
#include <SDL2/SDL.h>
#endif

/* 屏幕宽高 */
/* 限制物体的运动空间 */
int SCREEN_WIDTH = 750, SCREEN_HEIGHT = 400;
const int  SCREEN_TICK_PER_FRAME = 1000 / 60;

/* 我们将渲染到的窗口 */
SDL_Window *gWindow;
/* 包含的窗口表面 */
SDL_Surface *gScreenSurface;

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

}
void close() {
	SDL_FreeSurface(gScreenSurface);
	SDL_DestroyWindow(gWindow);
	SDL_Quit();
}

typedef struct _Point {
	float x, y;
}Point;

struct  GameObject {
	/*  设置恢复原状，较低的值会在碰撞时损失更多的能量 */
	const float restitution = 0.91;
	/* 设置默认宽度和高度 */
	int mWidth, mHeight;
	float mass;
	float radius;
	float x, y;
	float vx, vy;
	int isColliding;
	SDL_Rect mousePos, clickOffset;
	bool leftMouseButtonDown, SelectRect;

	GameObject(float x, float y, float vx, float vy, float r, float m)
		:x(x), y(y), vx(vx), vy(vy), radius(r), mass(m), isColliding(0), mWidth(r * 2), mHeight(r * 2) {
		leftMouseButtonDown = SelectRect = 0;
	}
	virtual ~GameObject() {
	}

	virtual SDL_Rect GetRect() = 0;
	virtual void draw() = 0;
	virtual void update(float) = 0;
	bool isRect(SDL_Rect a) {
		//SDL_PointInRect()
		return ((a.x >= x - mWidth / 2) && (a.x <= (x + mWidth / 2)) &&
			(a.y >= y - mHeight / 2) && (a.y <= (y + mHeight / 2)));
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
					SelectRect = true;
				}
			}
			else if (e.type == SDL_MOUSEMOTION)
			{
				if (leftMouseButtonDown&&SelectRect)
				{
					isColliding = 3;
					x = mousePos.x - clickOffset.x;
					y = mousePos.y - clickOffset.y;
				}
			}
		}
	}
};

struct  Square :public GameObject {

	int color[4] = { 0x99b0,0xff8080,0xff,0xff00 };
public:
	Square(float x, float y, float vx, float vy, float r, float m) :GameObject(x, y, vx, vy, r, m) {
	}
	/*布雷森漢姆直線演算法 */
	void draw_line(int x1, int y1, int x2, int y2)
	{
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
		/* 画一个圆 */
		drawCircle();
		/* 绘制一条直线 */
		draw_line(x, y, x + vx, y + vy);;
	}
	/* 重力加速度的数值约为9.81米/秒^2 */
	const float g = pow(9.81, 3);
	void update(float secondsPassed) {
		if (isColliding > 1)
			return;
		/* 在地球上大约是每秒 9.81 米。您可以在游戏对象的update()函数中应用它。
		每一秒，都会在y速度上加上g，这会使物体下落的速度越来越快。 */
		if (isColliding != 1)
			vy += (g * secondsPassed);

		/* 以设定速度移动 */
		x += (vx * secondsPassed);
		y += (vy * secondsPassed);
		//cout << "x = " << x << "\t y = " << y << endl;

		/* 计算角度（vx 之前的vy） */
		float radians = atan2(vy, vx);
		/* 转换为度 */
		float degrees = 180 * radians / 3.14;
	}
};

float timeStamp = 0;
float secondsPassed = 0;
float oldTimeStamp = 0;
float movingSpeed = 50;

void gameLoop(vector<GameObject *>&gameObjects)
{
	/* 让我们再解释一下。当您的游戏以 60fps 运行时，大约是每帧 0.0167 秒。这意味着当您想要以每秒 50 像
		素的速度移动对象时，您必须将 50 乘以自上一帧以来经过的秒数。以 60 fps 运行的游戏将每帧移动对象
		0.835 像素。这就是update()函数中发生的事情。
		当帧率增加或减少时，移动速度也会增加。无论过去了多少时间，您的物体总是会以所需的速度移动。这
		使得动画更适合不同类型的硬件，具有不同的帧速率。当然，较低的帧率会使动画看起来不连贯，但物体
		的位移保持不变。 */
		/* 计算已经过去了多少时间 */
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
bool checkCollision(SDL_Rect a, SDL_Rect b, float ar, float br) {
	float squareDistance = (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
	return squareDistance <= (ar + br)*(ar + br);
}
void detectCollisions(vector<GameObject *>&gameObjects) {


	// 重置所有对象的碰撞状态
	for (size_t i = 0; i < gameObjects.size(); i++)
	{
		if (gameObjects[i]->isColliding <= 1)
			gameObjects[i]->isColliding = 0;
		else
			return;
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

				/* 带有长度和方向的箭头 */
				Point vCollision = { obj2.x - obj1.x,obj2.y - obj1.y };
				/* 两点之间的距离 */
				float distance = sqrt((obj2.x - obj1.x) * (obj2.x - obj1.x) + (obj2.y - obj1.y) * (obj2.y - obj1.y));
				/* 计算归一化的碰撞向量,就得到了方向 */
				Point vCollisionNorm = { vCollision.x / distance,vCollision.y / distance };
				/* 计算碰撞速度 */
				Point vRelativeVelocity = { obj1.vx - obj2.vx,obj1.vy - obj2.vy };
				float speed = vRelativeVelocity.x * vCollisionNorm.x + vRelativeVelocity.y * vCollisionNorm.y;

				if (speed < 0.1)
					break;

				speed *= min(obj1.restitution, obj2.restitution);
				/* 碰撞脉冲来将质量带入方程式 */
				float impulse = 2 * speed / (obj1.mass + obj2.mass);
				cout << impulse << endl;
				/*应用物理学，并通过根据速度计算碰撞脉冲来将质量带入方程。用冲量来计算动量。重的物体会把轻的推到一边。 */
				obj1.vx -= (impulse * obj2.mass * vCollisionNorm.x);
				obj1.vy -= (impulse * obj2.mass * vCollisionNorm.y);
				obj2.vx += (impulse * obj1.mass * vCollisionNorm.x);
				obj2.vy += (impulse * obj1.mass * vCollisionNorm.y);
			}
		}
	}
}


int main(int argc, char **argv) {
	init();
	/* 控制帧数 */
	double fpsTimer = 0, capTimer = 0;

	vector<GameObject *> gameObjects;
	/* 坐标X,Y,速度VX,VY,半径。质量。。*/
	gameObjects.push_back(new Square(250, 50, 0, 50, 25, 1));
	gameObjects.push_back(new Square(370, 0, 0, -50, 25, 2));
	gameObjects.push_back(new Square(150, 30, 50, 50, 25, 1));
	gameObjects.push_back(new Square(350, 75, -50, 50, 25, 1));
	gameObjects.push_back(new Square(300, 300, 50, -50, 25, 1));
	gameObjects.push_back(new Square(400, 300, 50, -50, 25, 1));
	gameObjects.push_back(new Square(400, 430, 50, -50, 25, 1));
	bool quit = false;
	SDL_Event e;

	while (!quit)
	{
		capTimer = SDL_GetTicks();

		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
				quit = true;
			for (auto& i : gameObjects)
				i->handleEvent(e);
		}


		/* 将窗口表面填充为白色*/
		SDL_FillRect(gScreenSurface, NULL, SDL_MapRGB(gScreenSurface->format, 255, 255, 255));

		gameLoop(gameObjects);
		detectEdgeCollisions(gameObjects);
		detectCollisions(gameObjects);
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
