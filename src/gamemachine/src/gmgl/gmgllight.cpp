﻿#include "stdafx.h"
#include "gmgllight.h"
#include "gmgltechniques.h"
#include "shader_constants.h"

template <GMuint32 i>
struct String {};

void GMGLLight::activateLight(GMuint32 index, ITechnique* technique)
{
	D(d);
	D_BASE(db, Base);

	GMGLTechnique* glTechnique = gm_cast<GMGLTechnique*>(technique);
	IShaderProgram* shaderProgram = glTechnique->getShaderProgram();
	GMString strIndex = GMString((GMint32)index);

	GMsize_t shaderLightIdx = verifyIndicesContainer(d->lightIndices, shaderProgram);
	static const Data::LightIndices s_empty = { 0 };
	if (d->lightIndices[shaderLightIdx].size() <= index)
		d->lightIndices[shaderLightIdx].resize(index + 1, s_empty);

	shaderProgram->setVec3(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].Color, L"GM_lights[" + strIndex + L"].Color"),
		db->color);

	shaderProgram->setVec3(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].Position, L"GM_lights[" + strIndex + L"].Position"),
		db->position);

	shaderProgram->setInt(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].Type, L"GM_lights[" + strIndex + L"].Type"),
		getType());

	shaderProgram->setVec3(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].AmbientIntensity, L"GM_lights[" + strIndex + L"].AmbientIntensity"),
		db->ambientIntensity);

	shaderProgram->setVec3(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].DiffuseIntensity, L"GM_lights[" + strIndex + L"].DiffuseIntensity"),
		db->diffuseIntensity);

	shaderProgram->setFloat(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].SpecularIntensity, L"GM_lights[" + strIndex + L"].SpecularIntensity"),
		db->specularIntensity);

	shaderProgram->setFloat(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].AttenuationConstant, L"GM_lights[" + strIndex + L"].Attenuation.Constant"),
		db->attenuation.constant);

	shaderProgram->setFloat(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].AttenuationLinear, L"GM_lights[" + strIndex + L"].Attenuation.Linear"),
		db->attenuation.linear);

	shaderProgram->setFloat(
		getVariableIndex(shaderProgram, d->lightIndices[shaderLightIdx][index].AttenuationExp, L"GM_lights[" + strIndex + L"].Attenuation.Exp"),
		db->attenuation.exp);
}