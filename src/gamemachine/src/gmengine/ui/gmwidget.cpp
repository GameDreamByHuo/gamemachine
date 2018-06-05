﻿#include "stdafx.h"
#include "gmwidget.h"
#include "gmcontrols.h"
#include "foundation/gamemachine.h"
#include "../gameobjects/gmgameobject.h"
#include "../gameobjects/gm2dgameobject.h"
#include "../gmtypoengine.h"
#include "gmdata/gmmodel.h"
#include "gmdata/glyph/gmglyphmanager.h"
#include "foundation/gmmessage.h"

namespace
{
	GMControl* getNextControl(GMControl* control)
	{
		GMWidget* parentCanvas = control->getParent();
		GMuint index = control->getIndex() + 1;

		// 如果下一个控件不在此画布内，则跳到下一个画布进行查找
		while (index >= (GMuint)parentCanvas->getControls().size())
		{
			parentCanvas = parentCanvas->getNextCanvas();
			index = 0;
		}

		return parentCanvas->getControls()[index];
	}

	GMControl* getPrevControl(GMControl* control)
	{
		GMWidget* parentCanvas = control->getParent();
		GMint index = (GMint) control->getIndex() - 1;
		while (index < 0)
		{
			parentCanvas = parentCanvas->getPrevCanvas();
			if (!parentCanvas)
				parentCanvas = control->getParent();
			index = parentCanvas->getControls().size() - 1;
		}
		return parentCanvas->getControls()[index];
	}

	GMsize_t indexOf(const Vector<GMWidget*>& widgets, GMWidget* targetCanvas)
	{
		for (GMsize_t i = 0; i < widgets.size(); ++i)
		{
			if (widgets[i] == targetCanvas)
				return i;
		}
		return -1;
	}

	bool operator <(const GMCanvasTextureInfo& lhs, const GMCanvasTextureInfo& rhs)
	{
		return lhs.texture < rhs.texture;
	}
}

void GMElementBlendColor::init(const GMVec4& defaultColor, const GMVec4& disabledColor, const GMVec4& hiddenColor)
{
	D(d);
	GM_FOREACH_ENUM_CLASS(state, GMControlState::Normal, GMControlState::EndOfControlState)
	{
		d->states[state] = defaultColor;
	};
	d->states[GMControlState::Disabled] = disabledColor;
	d->states[GMControlState::Hidden] = hiddenColor;
	d->current = hiddenColor;
}

void GMElementBlendColor::blend(GMControlState::State state, GMfloat elapsedTime, GMfloat rate)
{
	D(d);
	GMVec4 destColor = d->states[state];
	d->current = Lerp(d->current, destColor, elapsedTime);
}

void GMStyle::setTexture(GMWidgetResourceManager::TextureType texture, const GMRect& rc, const GMVec4& defaultTextureColor)
{
	D(d);
	d->texture = texture;
	d->rc = rc;
	d->textureColor.init(defaultTextureColor);
}

void GMStyle::setFont(GMuint font, const GMVec4& defaultColor)
{
	D(d);
	d->font = font;
	d->fontColor.init(defaultColor);
}

void GMStyle::setFontColor(GMControlState::State state, const GMVec4& color)
{
	D(d);
	d->fontColor.getStates()[state] = color;
}

void GMStyle::setTextureColor(GMControlState::State state, const GMVec4& color)
{
	D(d);
	d->textureColor.getStates()[state] = color;
}

void GMStyle::refresh()
{
	D(d);
	d->textureColor.setCurrent(d->textureColor.getStates()[GMControlState::Hidden]);
	d->fontColor.setCurrent(d->fontColor.getStates()[GMControlState::Hidden]);
}

GMWidgetResourceManager::GMWidgetResourceManager(const IRenderContext* context)
{
	D(d);
	d->context = context;
	d->screenQuadModel = new GMModel();
	d->screenQuadModel->setPrimitiveTopologyMode(GMTopologyMode::TriangleStrip);
	d->screenQuadModel->setUsageHint(GMUsageHint::DynamicDraw);
	d->screenQuadModel->setType(GMModelType::Model2D);
	GMMesh* mesh = new GMMesh(d->screenQuadModel);

	// 增加4个空白顶点
	mesh->vertex(GMVertex());
	mesh->vertex(GMVertex());
	mesh->vertex(GMVertex());
	mesh->vertex(GMVertex());

	d->screenQuad = new GMGameObject(GMAssets::createIsolatedAsset(GMAssetType::Model, d->screenQuadModel));
	d->screenQuad->setContext(getContext());
	GM.createModelDataProxyAndTransfer(d->context, d->screenQuadModel);

	d->textObject = new GMTextGameObject(context->getWindow()->getRenderRect());
	d->textObject->setContext(context);

	d->spriteObject = new GMSprite2DGameObject(context->getWindow()->getRenderRect());
	d->spriteObject->setContext(context);

	d->borderObject = new GMBorder2DGameObject(context->getWindow()->getRenderRect());
	d->borderObject->setContext(context);
}

GMWidgetResourceManager::~GMWidgetResourceManager()
{
	D(d);
	GM_delete(d->textObject);
	GM_delete(d->spriteObject);

	GM_delete(d->screenQuad);
	GM_delete(d->screenQuadModel);
}

GMModel* GMWidgetResourceManager::getScreenQuadModel()
{
	D(d);
	return d->screenQuadModel;
}

GMGameObject* GMWidgetResourceManager::getScreenQuad()
{
	D(d);
	return d->screenQuad;
}

void GMWidgetResourceManager::addTexture(TextureType type, ITexture* texture, GMint width, GMint height)
{
	D(d);
	GMCanvasTextureInfo texInfo;
	texInfo.texture = texture;
	texInfo.width = width;
	texInfo.height = height;

	d->textureResources[type] = texInfo;
}

void GMWidgetResourceManager::registerWidget(GMWidget* widget)
{
	D(d);
	for (auto c : d->widgets)
	{
		if (c == widget)
			return;
	}

	// 将Canvas设置成一个环
	d->widgets.push_back(widget);
	GMsize_t sz = d->widgets.size();
	if (sz > 1)
		d->widgets[sz - 2]->setNextCanvas(widget);
	d->widgets[sz - 1]->setNextCanvas(d->widgets[0]);
}

const GMCanvasTextureInfo& GMWidgetResourceManager::getTexture(TextureType type)
{
	D(d);
	return d->textureResources[type];
}

void GMWidgetResourceManager::onRenderRectResized()
{
	D(d);
	const GMWindowStates& windowStates = d->context->getWindow()->getWindowStates();
	d->backBufferWidth = windowStates.renderRect.width;
	d->backBufferHeight = windowStates.renderRect.height;
}

GMfloat GMWidget::s_timeRefresh = 0;
GMControl* GMWidget::s_controlFocus = nullptr;
GMControl* GMWidget::s_controlPressed = nullptr;

GMWidget::GMWidget(GMWidgetResourceManager* manager)
{
	D(d);
	d->manager = manager;
	d->nextCanvas = d->prevCanvas = this;
}

GMWidget::~GMWidget()
{
	removeAllControls();
}

void GMWidget::addControl(GMControl* control)
{
	D(d);
	d->controls.push_back(control);
	bool b = initControl(control);
	GM_ASSERT(b);
}

const GMRect& GMWidget::getArea(GMTextureArea::Area area)
{
	D(d);
	return d->areas[area];
}

void GMWidget::setTitleVisible(
	bool visible
)
{
	D(d);
	d->title = visible;
}

void GMWidget::setTitle(
	const GMString& text
)
{
	D(d);
	d->titleText = text;
}

void GMWidget::addStatic(
	const GMString& text,
	GMint x,
	GMint y,
	GMint width,
	GMint height,
	bool isDefault,
	OUT GMControlStatic** out
)
{
	GMControlStatic* staticControl = new GMControlStatic(this);
	if (out)
		*out = staticControl;

	addControl(staticControl);
	staticControl->setText(text);
	staticControl->setPosition(x, y);
	staticControl->setSize(width, height);
	staticControl->setIsDefault(isDefault);
}

void GMWidget::addButton(
	const GMString& text,
	GMint x,
	GMint y,
	GMint width,
	GMint height,
	bool isDefault,
	OUT GMControlButton** out
)
{
	GMControlButton* buttonControl = new GMControlButton(this);
	if (out)
		*out = buttonControl;

	addControl(buttonControl);
	buttonControl->setText(text);
	buttonControl->setPosition(x, y);
	buttonControl->setSize(width, height);
	buttonControl->setIsDefault(isDefault);
}

void GMWidget::addBorder(
	GMint x,
	GMint y,
	GMint width,
	GMint height,
	const GMRect& cornerRect,
	OUT GMControlBorder** out
)
{
	GMControlBorder* borderControl = new GMControlBorder(this);
	if (out)
		*out = borderControl;

	addControl(borderControl);
	borderControl->setPosition(x, y);
	borderControl->setSize(width, height);
	borderControl->setCorner(cornerRect);
}

void GMWidget::drawText(
	const GMString& text,
	GMStyle& style,
	const GMRect& rc,
	bool shadow,
	bool center
)
{
	// 不需要绘制透明元素
	if (style.getFontColor().getCurrent().getW() == 0)
		return;

	D(d);
	GMRect targetRc = rc;
	mapRect(targetRc);

	// TODO 先不考虑阴影什么的
	const GMVec4& fontColor = style.getFontColor().getCurrent();
	GMTextGameObject* textObject = d->manager->getTextObject();
	textObject->setColorType(Plain);
	textObject->setColor(fontColor);
	textObject->setText(text);
	textObject->setGeometry(targetRc);
	textObject->setCenter(center);
	textObject->draw();
}

void GMWidget::drawSprite(
	GMStyle& style,
	const GMRect& rc,
	GMfloat depth
)
{
	// 不需要绘制透明元素
	if (style.getTextureColor().getCurrent().getW() == 0)
		return;

	D(d);
	GMRect targetRc = rc;
	mapRect(targetRc);

	const GMRect& textureRc = style.getTextureRect();
	GMuint texId = style.getTexture();
	const GMCanvasTextureInfo& texInfo = d->manager->getTexture(style.getTexture());

	GMSprite2DGameObject* spriteObject = d->manager->getSpriteObject();
	spriteObject->setDepth(depth);
	spriteObject->setGeometry(targetRc);
	spriteObject->setTexture(texInfo.texture);
	spriteObject->setTextureRect(textureRc);
	spriteObject->setTextureSize(texInfo.width, texInfo.height);
	spriteObject->setColor(style.getTextureColor().getCurrent());
	spriteObject->draw();
}

void GMWidget::drawBorder(
	GMStyle& style,
	const GMRect& cornerRc,
	const GMRect& rc,
	GMfloat depth
)
{
	if (style.getTextureColor().getCurrent().getW() == 0)
		return;

	D(d);
	GMRect targetRc = rc;
	mapRect(targetRc);

	const GMRect& textureRc = style.getTextureRect();
	GMuint texId = style.getTexture();
	const GMCanvasTextureInfo& texInfo = d->manager->getTexture(style.getTexture());

	GMBorder2DGameObject* borderObject = d->manager->getBorderObject();
	borderObject->setDepth(depth);
	borderObject->setGeometry(targetRc);
	borderObject->setTexture(texInfo.texture);
	borderObject->setTextureRect(textureRc);
	borderObject->setTextureSize(texInfo.width, texInfo.height);
	borderObject->setColor(style.getTextureColor().getCurrent());
	borderObject->setCornerRect(cornerRc);
	borderObject->draw();
}

void GMWidget::requestFocus(GMControl* control)
{
	if (s_controlFocus == control)
		return;

	if (!control->canHaveFocus())
		return;

	if (s_controlFocus)
		s_controlFocus->onFocusOut();

	control->onFocusIn();
	s_controlFocus = control;
}

bool GMWidget::initControl(GMControl* control)
{
	D(d);
	GM_ASSERT(control);
	if (!control)
		return false;

	control->setIndex(d->controls.size() - 1);
	control->initStyles();
	return control->onInit();
}

void GMWidget::setPrevCanvas(GMWidget* prevCanvas)
{
	D(d);
	d->prevCanvas = prevCanvas;
}

void GMWidget::init()
{
	initStyles();
}

void GMWidget::addArea(GMTextureArea::Area area, const GMRect& rc)
{
	D(d);
	d->areas[area] = rc;
}

bool GMWidget::msgProc(GMSystemEvent* event)
{
	D(d);
	if (!getVisible())
		return false;

	GMSystemEventType type = event->getType();
	switch (type)
	{
	case GMSystemEventType::KeyDown:
	case GMSystemEventType::KeyUp:
	{
		GMSystemKeyEvent* keyEvent = gm_cast<GMSystemKeyEvent*>(event);
		if (s_controlFocus &&
			s_controlFocus->getParent() == this &&
			s_controlFocus->getEnabled())
		{
			if (s_controlFocus->handleKeyboard(keyEvent))
				return true;
		}

		if (type == GMSystemEventType::KeyDown)
		{
			if (!d->keyboardInput)
				return false;

			switch (keyEvent->getKey())
			{
			case GMKey_Right:
			case GMKey_Down:
			{
				if (s_controlFocus)
					return onCycleFocus(true);
				break;
			}
			case GMKey_Left:
			case GMKey_Up:
			{
				if (s_controlFocus)
					return onCycleFocus(false);
				break;
			}
			case GMKey_Tab:
				bool shiftDown = (keyEvent->getModifier() & GMModifier_Shift) != 0;
				return onCycleFocus(!shiftDown);
			}
		}
		break;
	}
	case GMSystemEventType::MouseDown:
	case GMSystemEventType::MouseUp:
	case GMSystemEventType::MouseMove:
	case GMSystemEventType::MouseDblClick:
	case GMSystemEventType::MouseWheel:
	{
		GMSystemMouseEvent* mouseEvent = gm_cast<GMSystemMouseEvent*>(event);
		GMPoint pt = mouseEvent->getPoint();
		pt.x -= d->x;
		pt.y -= d->y;

		GMSystemMouseWheelEvent cacheWheelEvent;
		GMSystemMouseEvent cacheEvent;

		GMSystemMouseEvent* pControlEvent = nullptr;
		if (type == GMSystemEventType::MouseWheel)
		{
			cacheWheelEvent = *(gm_cast<GMSystemMouseWheelEvent*>(mouseEvent));
			cacheWheelEvent.setPoint(pt);
			pControlEvent = &cacheWheelEvent;
		}
		else
		{
			cacheEvent = *mouseEvent;
			cacheEvent.setPoint(pt);
			pControlEvent = &cacheEvent;
		}

		// 判断是否拖拽Widget
		if (type == GMSystemEventType::MouseDown)
		{
			if (d->title)
			{
				const GMPoint& pt = cacheEvent.getPoint();
				GMRect rcBound = { 0, -d->titleHeight, d->width, d->titleHeight };
				if (GM_inRect(rcBound, pt) && onTitleMouseDown(&cacheEvent))
					return true;
			}
		}
		else if (type == GMSystemEventType::MouseMove)
		{
			if (d->title && onTitleMouseMove(&cacheEvent))
				return true;
		}
		else if (type == GMSystemEventType::MouseUp)
		{
			if (d->title && onTitleMouseUp(&cacheEvent))
				return true;
		}

		if (s_controlFocus &&
			s_controlFocus->getParent() == this &&
			s_controlFocus->getEnabled())
		{
			if (s_controlFocus->handleMouse(pControlEvent))
				return true;
		}

		// 点击测试，找到鼠标所在位置的控件
		GMControl* control = getControlAtPoint(pt);
		if (control && control->getEnabled())
		{
			if (control->handleMouse(pControlEvent))
				return true;
		}
		else
		{
			// 如果没有找到任何控件，那么当前的焦点控件失去焦点
			if (type == GMSystemEventType::MouseDown && 
				s_controlFocus &&
				s_controlFocus->getParent() == this)
			{
				s_controlFocus->onFocusOut();
				s_controlFocus = nullptr;
			}
		}

		// 仍然没有处理
		if (type == GMSystemEventType::MouseMove)
		{
			onMouseMove(pt);
			return false;
		}
		break;
	}
	}
	return false;
}

bool GMWidget::onTitleMouseDown(const GMSystemMouseEvent* event)
{
	D(d);
	if (!d->movingWidget)
	{
		d->movingWidget = true;
		d->movingStartPt = event->getPoint();
		return true;
	}
	return false;
}

bool GMWidget::onTitleMouseMove(const GMSystemMouseEvent* event)
{
	D(d);
	if (d->movingWidget)
	{
		const GMPoint& pt = event->getPoint();
		GMPoint deltaDistance = { pt.x - d->movingStartPt.x, pt.y - d->movingStartPt.y };
		d->x += deltaDistance.x;
		d->y += deltaDistance.y;
		return true;
	}
	return false;
}

bool GMWidget::onTitleMouseUp(const GMSystemMouseEvent* event)
{
	D(d);
	if (d->movingWidget)
		d->movingWidget = false;
	return false;
}

void GMWidget::onRenderTitle()
{
	D(d);
	GMRect rc = { 0, -d->titleHeight, d->width, d->titleHeight };
	drawSprite(d->titleStyle, rc, .99f);
	rc.x += 5;
	drawText(d->titleText, d->titleStyle, rc);
}

void GMWidget::render(GMfloat elpasedTime)
{
	D(d);
	if (d->timeLastRefresh < s_timeRefresh)
	{
		d->timeLastRefresh = GM.getGameMachineRunningStates().elapsedTime;
		refresh();
	}

	if (!d->visible ||
		(d->minimized && !d->title))
		return;

	bool backgroundVisible =
		d->colorTopLeft[2] > 0 ||
		d->colorTopRight[2] > 0 ||
		d->colorBottomLeft[2] > 0 ||
		d->colorBottomRight[2] > 0;

	if (!d->minimized && backgroundVisible)
	{
		GMfloat left, right, top, bottom;
		left = d->x * 2.0f / d->manager->getBackBufferWidth() - 1.0f;
		right = (d->x + d->width) * 2.0f / d->manager->getBackBufferWidth() - 1.0f;
		top = 1.0f - d->y * 2.0f / d->manager->getBackBufferHeight();
		bottom = 1.0f - (d->y + d->height) * 2.0f / d->manager->getBackBufferHeight();

		GMVertex vertices[4] = {
			{ { left,  top,    .5f }, { 0 }, { 0, 0 }, { 0 }, { 0 }, { 0 }, { d->colorTopLeft[0],     d->colorTopLeft[1],     d->colorTopLeft[2],     d->colorTopLeft[3]     } },
			{ { right, top,    .5f }, { 0 }, { 1, 0 }, { 0 }, { 0 }, { 0 }, { d->colorTopRight[0],    d->colorTopRight[1],    d->colorTopRight[2],    d->colorTopRight[3]    } },
			{ { left,  bottom, .5f }, { 0 }, { 0, 1 }, { 0 }, { 0 }, { 0 }, { d->colorBottomLeft[0],  d->colorBottomLeft[1],  d->colorBottomLeft[2],  d->colorBottomLeft[3]  } },
			{ { right, bottom, .5f }, { 0 }, { 1, 1 }, { 0 }, { 0 }, { 0 }, { d->colorBottomRight[0], d->colorBottomRight[1], d->colorBottomRight[2], d->colorBottomRight[3] } },
		};

		GMModel* quad = d->manager->getScreenQuadModel();
		GMModelDataProxy* proxy = quad->getModelDataProxy();
		// 使用proxy来更新其顶点
		proxy->beginUpdateBuffer();
		void* buffer = proxy->getBuffer();
		GM_ASSERT(buffer);
		memcpy_s(buffer, sizeof(vertices), &vertices, sizeof(vertices));
		proxy->endUpdateBuffer();

		// 开始绘制背景
		d->manager->getScreenQuad()->draw();
	}

	// TODO getTextureNode

	if (d->title)
	{
		onRenderTitle();
	}

	if (!d->minimized)
	{
		for (auto control : d->controls)
		{
			// 最后渲染焦点控件
			if (control == d->focusControl)
				continue;

			control->render(elpasedTime);
		}

		if (d->focusControl && d->focusControl->getParent() == this)
			d->focusControl->render(elpasedTime);
	}
}

void GMWidget::setNextCanvas(GMWidget* nextCanvas)
{
	D(d);
	if (!nextCanvas)
		nextCanvas = this;
	d->nextCanvas = nextCanvas;
	if (nextCanvas)
		nextCanvas->setPrevCanvas(this);
}

void GMWidget::refresh()
{
	D(d);
	if (s_controlFocus)
		s_controlFocus->onFocusOut();

	if (d->controlMouseOver)
		d->controlMouseOver->onMouseLeave();

	s_controlFocus = nullptr;
	s_controlPressed = nullptr;
	d->controlMouseOver = nullptr;

	for (auto control : d->controls)
	{
		control->refresh();
	}

	if (d->keyboardInput)
		focusDefaultControl();
}

void GMWidget::focusDefaultControl()
{
	D(d);
	for (auto& control : d->controls)
	{
		if (control->isDefault())
		{
			clearFocus();

			s_controlFocus = control;
			s_controlFocus->onFocusIn();
			return;
		}
	}
}

void GMWidget::removeAllControls()
{
	D(d);
	if (s_controlFocus && s_controlFocus->getParent() == this)
		s_controlFocus = nullptr;
	if (s_controlPressed && s_controlPressed->getParent() == this)
		s_controlPressed = nullptr;
	d->controlMouseOver = nullptr;

	for (auto control : d->controls)
	{
		GM_delete(control);
	}
	GMClearSTLContainer(d->controls);
}

GMControl* GMWidget::getControlAtPoint(const GMPoint& pt)
{
	D(d);
	for (auto control : d->controls)
	{
		if (!control)
			continue;

		if (control->getEnabled() && control->getVisible() && control->containsPoint(pt))
			return control;
	}
	return nullptr;
}

bool GMWidget::onCycleFocus(bool goForward)
{
	D(d);
	GMControl* control = nullptr;
	GMWidget* widget = nullptr;
	GMWidget* lastCanvas = nullptr;
	const Vector<GMWidget*>& widgets = d->manager->getCanvases();
	GMint sz = (GMint)widgets.size();

	if (!s_controlFocus)
	{
		if (goForward)
		{
			for (GMint i = 0; i < sz; ++i)
			{
				widget = lastCanvas = widgets[i];
				const Vector<GMControl*> controls = widget->getControls();
				if (widget && controls.size() > 0)
				{
					control = controls[0];
					break;
				}
			}
		}
		else
		{
			for (GMint i = sz - 1; i >= 0; --i)
			{
				widget = lastCanvas = widgets[i];
				const Vector<GMControl*> controls = widget->getControls();
				if (widget && controls.size() > 0)
				{
					control = controls[controls.size() - 1];
					break;
				}
			}
		}

		if (!widget || !control)
			return true;
	}
	else if (s_controlFocus->getParent() != this)
	{
		// 当前获得焦点的控件属于另外一个widget，所以要交给它来处理
		return false;
	}
	else
	{
		lastCanvas = s_controlFocus->getParent();
		control = (goForward) ? getNextControl(s_controlFocus) : getPrevControl(s_controlFocus);
		widget = control->getParent();
	}

	while (true)
	{
		// 如果我们转了一圈回来，那么我们不会设置任何焦点了。
		const Vector<GMWidget*> widgets = d->manager->getCanvases();
		if (widgets.empty())
			return false;

		GMsize_t lastCanvasIndex = indexOf(widgets, lastCanvas);
		GMsize_t canvasIndex = indexOf(widgets, widget);
		if ((!goForward && lastCanvasIndex < canvasIndex) ||
			(goForward && canvasIndex < lastCanvasIndex))
		{
			if (s_controlFocus)
				s_controlFocus->onFocusOut();
			s_controlFocus = nullptr;
			return true;
		}

		if (control == s_controlFocus)
			return true;

		if (control->getParent()->canKeyboardInput() && control->canHaveFocus())
		{
			if (s_controlFocus)
				s_controlFocus->onFocusOut();
			s_controlFocus = control;
			s_controlFocus->onFocusIn();
			return true;
		}

		lastCanvas = widget;
		control = (goForward) ? getNextControl(control) : getPrevControl(control);
		widget = control->getParent();
	}

	// 永远都不会到这里来，因为widget是个环，只会在上面return
	GM_ASSERT(false);
	return false;
}

void GMWidget::onMouseMove(const GMPoint& pt)
{
	D(d);
	GMControl* control = getControlAtPoint(pt);
	// 停留在相同控件上，不需要触发什么事件
	if (control == d->controlMouseOver)
		return;

	if (d->controlMouseOver)
		d->controlMouseOver->onMouseLeave();

	d->controlMouseOver = control;
	if (control)
		control->onMouseEnter();
}

void GMWidget::mapRect(GMRect& rc)
{
	D(d);
	rc.x += d->x;
	rc.y += d->y;
}

void GMWidget::initStyles()
{
	D(d);
	GMStyle titleStyle;
	titleStyle.setFont(0);
	titleStyle.setTexture(GMWidgetResourceManager::Skin, getArea(GMTextureArea::CaptionArea) );
	titleStyle.setTextureColor(GMControlState::Normal, GMVec4(1, 1, 1, 1));
	titleStyle.setFontColor(GMControlState::Normal, GMVec4(1, 1, 1, 1));
	titleStyle.getTextureColor().blend(GMControlState::Normal, .5f);
	titleStyle.getFontColor().blend(GMControlState::Normal, .5f);
	d->titleStyle = titleStyle;
}

void GMWidget::clearFocus()
{
	if (s_controlFocus)
	{
		s_controlFocus->onFocusOut();
		s_controlFocus = nullptr;
	}

	// TODO ReleaseCapture()
}