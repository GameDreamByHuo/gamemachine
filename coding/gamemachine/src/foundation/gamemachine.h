﻿#ifndef __GAMEMACHINE_H__
#define __GAMEMACHINE_H__
#include <gmcommon.h>
#include <utilities.h>
#include "../gmdatacore/glyph/gmglyphmanager.h"
#include "../gmdatacore/gamepackage/gmgamepackage.h"
#include "../os/gminput.h"
#include "../gmengine/gmcamera.h"
#include "gmthreads.h"

BEGIN_NS

#define GM GameMachine::instance()

GM_PRIVATE_OBJECT(GameMachine)
{
	GMClock clock;

	IWindow* mainWindow = nullptr;
	IFactory* factory = nullptr;
	IGraphicEngine* engine = nullptr;
	GMGlyphManager* glyphManager = nullptr;
	GMGamePackage* gamePackageManager = nullptr;
	IInput* inputManager = nullptr;
	GMStates* statesManager = nullptr;
	IGameHandler* gameHandler = nullptr;
	IWindow* consoleWindow = nullptr; // 内置调试窗口

	GMCamera camera;
	Queue<GameMachineMessage> messageQueue;
	Vector<IDispose*> manangerQueue;

	GMfloat lastFrameElpased = 0;
};

class GameMachine : public GMSingleton<GameMachine>
{
	DECLARE_PRIVATE(GameMachine)
	DECLARE_SINGLETON(GameMachine)

	enum
	{
		MAX_KEY_STATE_BITS = 512,
	};

public:
	enum EndiannessMode
	{
		// Never returns:
		UNKNOWN_YET = -1,

		// Modes:
		LITTLE_ENDIAN = 0,
		BIG_ENDIAN = 1,
	};

public:
	GameMachine() = default;

public:
	void init(
		AUTORELEASE IWindow* mainWindow,
		AUTORELEASE IWindow* consoleWindow,
		AUTORELEASE IFactory* factory,
		AUTORELEASE IGameHandler* gameHandler
	);

	// 绘制引擎
	inline IGraphicEngine* getGraphicEngine() { D(d); return d->engine; }

	// 主窗口
	inline IWindow* getMainWindow() { D(d); return d->mainWindow; }

	// 工厂管理
	inline IFactory* getFactory() { D(d); return d->factory; }

	// 配置管理
	GMStates* getStatesManager() { D(d); return d->statesManager; }

	// 字体管理
	GMGlyphManager* getGlyphManager() { D(d); return d->glyphManager; }

	// 资源管理
	GMGamePackage* getGamePackageManager() { D(d); return d->gamePackageManager; }

	// HID
	IInput* getInputManager() { D(d); return d->inputManager; }

	// 相机管理
	GMCamera& getCamera() { D(d); return d->camera; }

	// 时间管理
	inline GMfloat getFPS() { D(d); return d->clock.getFps(); }
	inline GMfloat getGameTimeSeconds() { D(d); return d->clock.getTime(); }
	inline GMfloat getLastFrameElapsed() { D(d); return d->lastFrameElpased; }

	// 绘制管理
	void initObjectPainter(GMModel* obj);

	// 大小端模式
	EndiannessMode getMachineEndianness();

	// 发送事件
	void postMessage(GameMachineMessage msg);

	void startGameMachine();

private:
	template <typename T, typename U> void registerManager(T* newObject, OUT U** manager);
	void terminate();
	bool handleMessages();
	void initInner();
};

END_NS
#endif
