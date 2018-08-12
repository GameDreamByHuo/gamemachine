﻿#include "stdafx.h"
#include "gmgameobject.h"
#include "gmengine/gmgameworld.h"
#include "gmdata/glyph/gmglyphmanager.h"
#include "foundation/gamemachine.h"
#include "gmassets.h"

GMGameObject::GMGameObject(GMAsset asset)
	: GMGameObject()
{
	addModel(asset, true);
	updateTransformMatrix();
}

void GMGameObject::addModel(GMAsset asset, bool replace)
{
	D(d);
	d->asset = asset;
	GMModels* models = asset.getModels();
	if (models)
	{
		if (replace)
		{
			d->models.swap(models);
		}
		else
		{
			for (decltype(auto) model : models->getModels())
			{
				d->models.push_back(model);
			}
		}
	}
	else
	{
		GMModel* model = asset.getModel();
		GM_ASSERT(model);
		d->models.push_back(model);
	}
}

GMModels& GMGameObject::getModels()
{
	D(d);
	return d->models;
}

void GMGameObject::setWorld(GMGameWorld* world)
{
	D(d);
	GM_ASSERT(!d->world);
	d->world = world;
}

GMGameWorld* GMGameObject::getWorld()
{
	D(d);
	return d->world;
}

void GMGameObject::setPhysicsObject(AUTORELEASE GMPhysicsObject* phyObj)
{
	D(d);
	d->physics.reset(phyObj);
	d->physics->setGameObject(this);
}

void GMGameObject::draw()
{
	const GMModels& models = getModels();
	for (decltype(auto) model : models.getModels())
	{
		drawModel(getContext(), model);
	}
}

bool GMGameObject::canDeferredRendering()
{
	D(d);
	for (decltype(auto) model : d->models.getModels())
	{
		if (model->getShader().getBlend() == true)
			return false;

		if (model->getShader().getVertexColorOp() == GMS_VertexColorOp::Replace)
			return false;
	}
	return true;
}

const IRenderContext* GMGameObject::getContext()
{
	D(d);
	return d->context;
}

void GMGameObject::updateTransformMatrix()
{
	D(d);
	d->transformMatrix = d->scaling * QuatToMatrix(d->rotation) * d->translation;
}

void GMGameObject::setScaling(const GMMat4& scaling)
{
	D(d);
	d->scaling = scaling;
	if (d->autoUpdateTransformMatrix)
		updateTransformMatrix();
}

void GMGameObject::setTranslation(const GMMat4& translation)
{
	D(d);
	d->translation = translation;
	if (d->autoUpdateTransformMatrix)
		updateTransformMatrix();
}

void GMGameObject::setRotation(const GMQuat& rotation)
{
	D(d);
	d->rotation = rotation;
	if (d->autoUpdateTransformMatrix)
		updateTransformMatrix();
}

void GMGameObject::beginUpdateTransform()
{
	setAutoUpdateTransformMatrix(false);
}

void GMGameObject::endUpdateTransform()
{
	setAutoUpdateTransformMatrix(true);
	updateTransformMatrix();
}

void GMGameObject::drawModel(const IRenderContext* context, GMModel* model)
{
	IGraphicEngine* engine = context->getEngine();
	if (model->getShader().getDiscard())
		return;

	ITechnique* technique = engine->getTechnique(model->getType());
	technique->beginModel(model, this);
	technique->draw(model);
	technique->endModel();
}

GMCubeMapGameObject::GMCubeMapGameObject(ITexture* texture)
{
	createCubeMap(texture);
}

void GMCubeMapGameObject::deactivate()
{
	D(d);
	GMGameWorld* world = getWorld();
	if (world)
		world->getContext()->getEngine()->update(GMUpdateDataType::TurnOffCubeMap);
}

void GMCubeMapGameObject::createCubeMap(ITexture* texture)
{
	GMfloat vertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,

		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,

		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,

		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,

		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	GMfloat v[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
	};

	GMModel* model = new GMModel();
	model->setType(GMModelType::CubeMap);
	model->getShader().getTextureList().getTextureSampler(GMTextureType::CubeMap).addFrame(GMAsset(GMAssetType::Texture, texture));
	GMMesh* mesh = new GMMesh(model);
	for (GMuint i = 0; i < 12; i++)
	{
		GMVertex V0 = { { vertices[i * 9 + 0], vertices[i * 9 + 1], vertices[i * 9 + 2] }, { vertices[i * 9 + 0], vertices[i * 9 + 1], vertices[i * 9 + 2] } };
		GMVertex V1 = { { vertices[i * 9 + 3], vertices[i * 9 + 4], vertices[i * 9 + 5] },{ vertices[i * 9 + 3], vertices[i * 9 + 4], vertices[i * 9 + 5] } };
		GMVertex V2 = { { vertices[i * 9 + 6], vertices[i * 9 + 7], vertices[i * 9 + 8] },{ vertices[i * 9 + 6], vertices[i * 9 + 7], vertices[i * 9 + 8] } };
		mesh->vertex(V0);
		mesh->vertex(V1);
		mesh->vertex(V2);
	}

	addModel(GMAsset(GMAssetType::Model, model));
}

bool GMCubeMapGameObject::canDeferredRendering()
{
	return false;
}
