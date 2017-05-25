﻿#include "stdafx.h"
#ifdef __APPLE__
#include <stdlib.h>
#endif
#include "bsp.h"
#include <stdio.h>
#include "foundation/utilities/utilities.h"
#include "bsp_interior.inl"
#include "gmdatacore/gamepackage.h"
#include "foundation/linearmath.h"

static const char* getValue(const BSPEntity* entity, const char* key)
{
	BSPKeyValuePair* e = entity->epairs;
	while (e)
	{
		if (strEqual(e->key, key))
			return e->value;
		e = e->next;
	}
	return nullptr;
}

#define Copy(dest, src) memcpy(dest, src, sizeof(src))
#define CopyMember(dest, src, prop) dest.prop = src.prop

//////////////////////////////////////////////////////////////////////////
BSP::BSP()
{
}

BSP::~BSP()
{
	D(d);
	delete[] d->buffer;
}

void BSP::loadBsp(const GamePackageBuffer& buf)
{
	D(d);
	d->buffer = new GMbyte[buf.size];
	memcpy(d->buffer, buf.buffer, buf.size);
	swapBsp();
	parseEntities();
	toGLCoord();
	generateLightVolumes();
}

BSPData& BSP::bspData()
{
	D(d);
	return *d;
}

void BSP::swapBsp()
{
	D(d);
	d->header = (BSPHeader*)d->buffer;
	SwapBlock((int *)d->header, sizeof(*d->header));

	if (d->header->ident != BSP_IDENT) {
		gm_error("Invalid IBSP file");
	}
	if (d->header->version != BSP_VERSION) {
		gm_error("Bad version of bsp.");
	}

	// 读取无对齐结构
	loadNoAlignData();

	// 以下结构有对齐，应该用特殊方式读取
	loadPlanes();
	loadVertices();
	loadDrawSurfaces();
}

void BSP::loadPlanes()
{
	D(d);
	struct __Tag
	{
		GMfloat p[4];
	};

	GMint length = (d->header->lumps[LUMP_PLANES].filelen) / sizeof(__Tag);
	if (length > 0)
	{
		AlignedVector<__Tag> t;
		t.resize(length);
		GMint num = CopyLump(d->header, LUMP_PLANES, &t[0], sizeof(__Tag));
		d->numplanes = num;
		d->planes.resize(length);
		for (GMint i = 0; i < num; i++)
		{
			d->planes[i].normal = linear_math::Vector3(t[i].p[0], t[i].p[1], t[i].p[2]);
			d->planes[i].intercept = t[i].p[3];
		}
	}
	else
	{
		d->numplanes = 0;
	}
}

void BSP::loadVertices()
{
	D(d);
	struct __Tag
	{
		GMfloat xyz[3];
		GMfloat st[2];
		GMfloat lightmap[2];
		GMfloat normal[3];
		GMbyte color[4];
	};

	GMint length = (d->header->lumps[LUMP_DRAWVERTS].filelen) / sizeof(__Tag);
	if (length > 0)
	{
		AlignedVector<__Tag> t;
		t.resize(length);
		GMint num = CopyLump(d->header, LUMP_DRAWVERTS, &t[0], sizeof(__Tag));
		d->numDrawVertices = num;
		d->vertices.resize(length);
		for (GMint i = 0; i < num; i++)
		{
			d->vertices[i].xyz = linear_math::Vector3(t[i].xyz[0], t[i].xyz[1], t[i].xyz[2]);
			d->vertices[i].normal = linear_math::Vector3(t[i].normal[0], t[i].normal[1], t[i].normal[2]);
			Copy(d->vertices[i].st, t[i].st);
			Copy(d->vertices[i].color, t[i].color);
			Copy(d->vertices[i].lightmap, t[i].lightmap);
		}
	}
	else
	{
		d->numDrawVertices = 0;
	}
}

void BSP::loadDrawSurfaces()
{
	D(d);
	struct __Tag
	{
		GMint shaderNum;
		GMint fogNum;
		GMint surfaceType;

		GMint firstVert;
		GMint numVerts;

		GMint firstIndex;
		GMint numIndexes;

		GMint lightmapNum;
		GMint lightmapX, lightmapY;
		GMint lightmapWidth, lightmapHeight;

		GMfloat lightmapOrigin[3];
		GMfloat lightmapVecs[3][3];

		GMint patchWidth;
		GMint patchHeight;
	};

	GMint length = (d->header->lumps[LUMP_SURFACES].filelen) / sizeof(__Tag);
	if (length > 0)
	{
		AlignedVector<__Tag> t;
		t.resize(length);
		GMint num = CopyLump(d->header, LUMP_SURFACES, &t[0], sizeof(__Tag));
		d->numDrawSurfaces = num;
		d->drawSurfaces.resize(length);
		for (GMint i = 0; i < num; i++)
		{
			d->drawSurfaces[i].lightmapOrigin = linear_math::Vector3(t[i].lightmapOrigin[0], t[i].lightmapOrigin[1], t[i].lightmapOrigin[2]);
			for (GMint j = 0; j < 3; j++)
			{
				d->drawSurfaces[i].lightmapVecs[j] = linear_math::Vector3(t[i].lightmapVecs[j][0], t[i].lightmapVecs[j][1], t[i].lightmapVecs[j][2]);
			}
			CopyMember(d->drawSurfaces[i], t[i], shaderNum);
			CopyMember(d->drawSurfaces[i], t[i], fogNum);
			CopyMember(d->drawSurfaces[i], t[i], surfaceType);

			CopyMember(d->drawSurfaces[i], t[i], firstVert);
			CopyMember(d->drawSurfaces[i], t[i], numVerts);

			CopyMember(d->drawSurfaces[i], t[i], firstIndex);
			CopyMember(d->drawSurfaces[i], t[i], numIndexes);

			CopyMember(d->drawSurfaces[i], t[i], lightmapNum);
			CopyMember(d->drawSurfaces[i], t[i], lightmapX);
			CopyMember(d->drawSurfaces[i], t[i], lightmapY);
			CopyMember(d->drawSurfaces[i], t[i], lightmapWidth);
			CopyMember(d->drawSurfaces[i], t[i], lightmapHeight);

			CopyMember(d->drawSurfaces[i], t[i], patchWidth);
			CopyMember(d->drawSurfaces[i], t[i], patchHeight);
		}
	}
	else
	{
		d->numDrawSurfaces = 0;
	}
}

void BSP::loadNoAlignData()
{
	D(d);
	const int extrasize = 1;
	GMint length = (d->header->lumps[LUMP_SHADERS].filelen) / sizeof(BSPShader);
	d->shaders.resize(length + extrasize);
	d->numShaders = CopyLump(d->header, LUMP_SHADERS, &d->shaders[0], sizeof(BSPShader));

	length = (d->header->lumps[LUMP_MODELS].filelen) / sizeof(BSPModel);
	d->models.resize(length + extrasize);
	d->nummodels = CopyLump(d->header, LUMP_MODELS, &d->models[0], sizeof(BSPModel));

	length = (d->header->lumps[LUMP_LEAFS].filelen) / sizeof(BSPLeaf);
	d->leafs.resize(length + extrasize);
	d->numleafs = CopyLump(d->header, LUMP_LEAFS, &d->leafs[0], sizeof(BSPLeaf));

	length = (d->header->lumps[LUMP_NODES].filelen) / sizeof(BSPNode);
	d->nodes.resize(length + extrasize);
	d->numnodes = CopyLump(d->header, LUMP_NODES, &d->nodes[0], sizeof(BSPNode));

	length = (d->header->lumps[LUMP_LEAFSURFACES].filelen) / sizeof(int);
	d->leafsurfaces.resize(length + extrasize);
	d->numleafsurfaces = CopyLump(d->header, LUMP_LEAFSURFACES, &d->leafsurfaces[0], sizeof(int));

	length = (d->header->lumps[LUMP_LEAFBRUSHES].filelen) / sizeof(int);
	d->leafbrushes.resize(length + extrasize);
	d->numleafbrushes = CopyLump(d->header, LUMP_LEAFBRUSHES, &d->leafbrushes[0], sizeof(int));

	length = (d->header->lumps[LUMP_BRUSHES].filelen) / sizeof(BSPBrush);
	d->brushes.resize(length + extrasize);
	d->numbrushes = CopyLump(d->header, LUMP_BRUSHES, &d->brushes[0], sizeof(BSPBrush));

	length = (d->header->lumps[LUMP_BRUSHSIDES].filelen) / sizeof(BSPBrushSide);
	d->brushsides.resize(length + extrasize);
	d->numbrushsides = CopyLump(d->header, LUMP_BRUSHSIDES, &d->brushsides[0], sizeof(BSPBrushSide));

	length = (d->header->lumps[LUMP_FOGS].filelen) / sizeof(BSPFog);
	d->fogs.resize(length + extrasize);
	d->numFogs = CopyLump(d->header, LUMP_FOGS, &d->fogs[0], sizeof(BSPFog));

	length = (d->header->lumps[LUMP_DRAWINDEXES].filelen) / sizeof(int);
	d->drawIndexes.resize(length + extrasize);
	d->numDrawIndexes = CopyLump(d->header, LUMP_DRAWINDEXES, &d->drawIndexes[0], sizeof(int));

	length = (d->header->lumps[LUMP_VISIBILITY].filelen) / 1;
	d->visBytes.resize(length + extrasize);
	d->numVisBytes = CopyLump(d->header, LUMP_VISIBILITY, &d->visBytes[0], 1);

	length = (d->header->lumps[LUMP_LIGHTMAPS].filelen) / 1;
	d->lightBytes.resize(length + extrasize);
	d->numLightBytes = CopyLump(d->header, LUMP_LIGHTMAPS, &d->lightBytes[0], 1);

	length = (d->header->lumps[LUMP_ENTITIES].filelen) / 1;
	d->entdata.resize(length + extrasize);
	d->entdatasize = CopyLump(d->header, LUMP_ENTITIES, &d->entdata[0], 1);

	length = (d->header->lumps[LUMP_LIGHTGRID].filelen) / 1;
	d->gridData.resize(length + extrasize);
	d->numGridPoints = CopyLump(d->header, LUMP_LIGHTGRID, &d->gridData[0], 8);

}

// 将坐标系转化为xyz(OpenGL)坐标系
// Quake的坐标系为：z正向朝上，y正向朝内，x正向朝右
void BSP::toGLCoord()
{
	D(d);
	for (GMint i = 0; i < d->numDrawVertices; i++)
	{
		//swap y and z and negate z
		GMfloat &_1 = d->vertices[i].xyz[1],
				&_2 = d->vertices[i].xyz[2];
		SWAP(_1, _2);
		_2 = -_2;


		//Transfer texture coordinates (Invert t)
		d->vertices[i].st[1] = -d->vertices[i].st[1];
	}

	for (GMint i = 0; i < d->numplanes; ++i)
	{
		//swap y and z and negate z
		SWAP(d->planes[i].normal[1], d->planes[i].normal[2]);
		d->planes[i].normal[2] = -d->planes[i].normal[2];
		d->planes[i].intercept = -d->planes[i].intercept;
	}
}

void BSP::parseEntities()
{
	D(d);
	d->lightVols.lightVolSize[0] = 64;
	d->lightVols.lightVolSize[1] = 64;
	d->lightVols.lightVolSize[2] = 128;

	parseFromMemory(d->entdata.data(), d->entdatasize);

	BSPEntity* entity;
	while ((entity = parseEntity()))
	{
		const char* gridSize = getValue(entity, "gridsize");
		if (gridSize)
		{
			Scanner s(gridSize);
			s.nextFloat(&d->lightVols.lightVolSize[0]);
			s.nextFloat(&d->lightVols.lightVolSize[1]);
			s.nextFloat(&d->lightVols.lightVolSize[2]);
			gm_info("gridSize reseted to %f, %f, %f", &d->lightVols.lightVolSize[0], &d->lightVols.lightVolSize[1], &d->lightVols.lightVolSize[2]);
		}
	}
}

void BSP::generateLightVolumes()
{
	D(d);
	d->lightVols.lightVolInverseSize = 1.f / d->lightVols.lightVolSize;
	GMfloat* wMins = d->models[0].mins;
	GMfloat* wMaxs = d->models[0].maxs;
	linear_math::Vector3 maxs;
	GMint numGridPoints = 0;

	for (GMuint i = 0; i < 3; i++)
	{
		d->lightVols.lightVolOrigin[i] = d->lightVols.lightVolSize[i] * ceil(wMins[i] / d->lightVols.lightVolSize[i]);
		maxs[i] = d->lightVols.lightVolSize[i] * floor(wMaxs[i] / d->lightVols.lightVolSize[i]);
		d->lightVols.lightVolBounds[i] = (maxs[i] - d->lightVols.lightVolOrigin[i]) / d->lightVols.lightVolSize[i] + 1;
	}
	numGridPoints = d->lightVols.lightVolBounds[0] * d->lightVols.lightVolBounds[1] * d->lightVols.lightVolBounds[2];

	if (d->header->lumps[LUMP_LIGHTGRID].filelen != numGridPoints * 8)
	{
		gm_warning("light volumes mismatch.");
		d->lightBytes.clear();
		return;
	}
}

void BSP::parseFromMemory(char *buffer, int size)
{
	D(d);
	d->script = d->scriptstack;
	d->script++;
	if (d->script == &d->scriptstack[MAX_INCLUDES])
		gm_error("script file exceeded MAX_INCLUDES");
	strcpy_s(d->script->filename, "memory buffer");

	d->script->buffer = buffer;
	d->script->line = 1;
	d->script->script_p = d->script->buffer;
	d->script->end_p = d->script->buffer + size;

	d->endofscript = false;
	d->tokenready = false;
}

BSPEntity* BSP::parseEntity()
{
	D(d);

	BSPKeyValuePair* e;
	BSPEntity *mapent;

	if (!getToken(true))
		return nullptr;

	if (!strEqual(d->token, "{")) {
		gm_warning("parseEntity: { not found");
	}

	BSPEntity bla;
	bla.epairs = 0;
	bla.firstDrawSurf = 0;
	bla.origin[0] = 0.f;
	bla.origin[1] = 0.f;
	bla.origin[2] = 0.f;
	mapent = &bla;
	do
	{
		if (!getToken(true)) {
			gm_warning("parseEntity: EOF without closing brace");
		}
		if (strEqual(d->token, "}")) {
			break;
		}
		e = (struct BSPPair*)parseEpair();
		e->next = mapent->epairs;
		mapent->epairs = e;

		if (strEqual(e->key, "origin"))
		{
			Scanner s(e->value);
			s.nextFloat(&bla.origin[0]); // x
			s.nextFloat(&bla.origin[1]); // z
			s.nextFloat(&bla.origin[2]); // y
			SWAP(bla.origin[1], bla.origin[2]);
			bla.origin[2] = -bla.origin[2];
		}
	} while (true);

	d->entities.push_back(bla);

	return &d->entities.back();
}

bool BSP::getToken(bool crossline)
{
	D(d);
	char *token_p;

	if (d->tokenready)                         // is a token allready waiting?
	{
		d->tokenready = false;
		return true;
	}

	if (d->script->script_p >= d->script->end_p)
		return endOfScript(crossline);

	//
	// skip space
	//
skipspace:
	while (*d->script->script_p <= 32)
	{
		if (d->script->script_p >= d->script->end_p)
			return endOfScript(crossline);
		if (*d->script->script_p++ == '\n')
		{
			if (!crossline)
				gm_error(false, "Line %i is incomplete\n", d->scriptline);
			d->scriptline = d->script->line++;
		}
	}

	if (d->script->script_p >= d->script->end_p)
		return endOfScript(crossline);

	// ; # // comments
	if (*d->script->script_p == ';' || *d->script->script_p == '#'
		|| (d->script->script_p[0] == '/' && d->script->script_p[1] == '/'))
	{
		if (!crossline)
			gm_error("Line %i is incomplete\n", d->scriptline);
		while (*d->script->script_p++ != '\n')
			if (d->script->script_p >= d->script->end_p)
				return endOfScript(crossline);
		d->scriptline = d->script->line++;
		goto skipspace;
	}

	// /* */ comments
	if (d->script->script_p[0] == '/' && d->script->script_p[1] == '*')
	{
		if (!crossline)
		{
			gm_error("Line %i is incomplete\n", d->scriptline);
			ASSERT(false);
		}
		d->script->script_p += 2;
		while (d->script->script_p[0] != '*' && d->script->script_p[1] != '/')
		{
			if (*d->script->script_p == '\n') {
				d->scriptline = d->script->line++;
			}
			d->script->script_p++;
			if (d->script->script_p >= d->script->end_p)
				return endOfScript(crossline);
		}
		d->script->script_p += 2;
		goto skipspace;
	}

	//
	// copy token
	//
	token_p = d->token;

	if (*d->script->script_p == '"')
	{
		// quoted token
		d->script->script_p++;
		while (*d->script->script_p != '"')
		{
			*token_p++ = *d->script->script_p++;
			if (d->script->script_p == d->script->end_p)
				break;
			if (token_p == &d->token[MAXTOKEN])
				gm_error("Token too large.");
		}
		d->script->script_p++;
	}
	else	// regular token
		while (*d->script->script_p > 32 && *d->script->script_p != ';')
		{
			*token_p++ = *d->script->script_p++;
			if (d->script->script_p == d->script->end_p)
				break;
			if (token_p == &d->token[MAXTOKEN])
				gm_error("Token too large on line %i\n", d->scriptline);
		}

	*token_p = 0;

	if (!strcmp(d->token, "$include"))
	{
		getToken(false);
		addScriptToStack(d->token);
		return getToken(crossline);
	}

	return true;
}

BSPKeyValuePair* BSP::parseEpair(void)
{
	D(d);
	BSPKeyValuePair	*e;

	e = (struct BSPPair*) malloc(sizeof(BSPKeyValuePair));
	memset(e, 0, sizeof(BSPKeyValuePair));

	if (strlen(d->token) >= MAX_KEY - 1) {
		//printf ("ParseEpar: token too long");
	}
	e->key = copystring(d->token);
	getToken(false);
	if (strlen(d->token) >= MAX_VALUE - 1) {

		//printf ("ParseEpar: token too long");
	}
	e->value = copystring(d->token);

	// strip trailing spaces that sometimes get accidentally
	// added in the editor
	stripTrailing(e->key);
	stripTrailing(e->value);

	return e;
}

void BSP::addScriptToStack(const char *filename)
{
	D(d);
	GMint size;

	d->script++;
	if (d->script == &d->scriptstack[MAX_INCLUDES])
		gm_error("script file exceeded MAX_INCLUDES");
	strcpy_s(d->script->filename, expandPath(filename).c_str());

	size = LoadFile(d->script->filename, (void **)&d->script->buffer);

	gm_info("entering %s\n", d->script->filename);

	d->script->line = 1;

	d->script->script_p = d->script->buffer;
	d->script->end_p = d->script->buffer + size;
}


bool BSP::endOfScript(bool crossline)
{
	D(d);
	if (!crossline)
	{
		gm_warning(false, "Line %i is incomplete\n", d->scriptline);
	}

	if (!strcmp(d->script->filename, "memory buffer"))
	{
		d->endofscript = true;
		return false;
	}

	free(d->script->buffer);
	if (d->script == d->scriptstack + 1)
	{
		d->endofscript = true;
		return false;
	}
	d->script--;
	d->scriptline = d->script->line;
	gm_info("returning to %s\n", d->script->filename);
	return getToken(crossline);
}
