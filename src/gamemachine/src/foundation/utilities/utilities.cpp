﻿#include "stdafx.h"
#include "utilities.h"
#include "foundation/gamemachine.h"
#include "gmengine/gmgameworld.h"
#include "gmdata/gmimagebuffer.h"
#include "gmengine/particle/gmparticle.h"

#define getVertex(x, y) (vertices[(x) + (y) * (sliceM + 1)])
#define getIndex(x, y) ((x) + (y) * (sliceM + 1))

namespace
{
	// 按照以下顺序求法线:
	// p0   p1 | p2   p0 |      p2 | p1
	//         |         |         |
	// p2      |      p1 | p1   p0 | p0   p2
	// 法线 = (p1 - p0) * (p2 - p0)
	GMVec3 calculateNormal(const GMVertex& p0, const GMVertex& p1, const GMVertex& p2)
	{
		GMVec3 v0 = GMVec3(p0.positions[0], p0.positions[1], p0.positions[2]);
		GMVec3 v1 = GMVec3(p1.positions[0], p1.positions[1], p1.positions[2]) - v0;
		GMVec3 v2 = GMVec3(p2.positions[0], p2.positions[1], p2.positions[2]) - v0;
		return Normalize(Cross(v1, v2));
	}

	enum VertexPosition
	{
		Left,
		Right,
		Up,
		Down,
	};

	GMVertex& getAdjacentVertex(Vector<GMVertex>& vertices, GMsize_t x, GMsize_t y, GMsize_t sliceM, VertexPosition pos)
	{
		static GMVertex s_invalid;
		switch (pos)
		{
		case Left:
			return getVertex(x - 1, y);
		case Right:
			return getVertex(x + 1, y);
		case Up:
			return getVertex(x, y + 1);
		case Down:
			return getVertex(x, y - 1);
		default:
			GM_ASSERT(false);
			return s_invalid;
		}
	}
}

const GMVec3& GMPrimitiveCreator::one3()
{
	static GMVec3 o (1, 1, 1);
	return o;
}

const GMVec2& GMPrimitiveCreator::one2()
{
	static GMVec2 o(1, 1);
	return o;
}

GMfloat* GMPrimitiveCreator::origin()
{
	static GMfloat o[3] = { 0 };
	return o;
}

void GMPrimitiveCreator::createCube(const GMVec3& halfExtents, REF GMModelAsset& model)
{
	static const GMVec3 s_vertices[8] = {
		GMVec3(-1, -1, -1),
		GMVec3(-1, 1, -1),
		GMVec3(1, -1, -1),
		GMVec3(1, 1, -1),

		GMVec3(-1, -1, 1),
		GMVec3(-1, 1, 1),
		GMVec3(1, -1, 1),
		GMVec3(1, 1, 1),
	};

	GMVec3 vertices_vec[8];
	GMFloat4 vertices[8];
	for (GMint i = 0; i < GM_array_size(vertices); ++i)
	{
		vertices_vec[i] = s_vertices[i] * halfExtents;
		vertices_vec[i].loadFloat4(vertices[i]);
	}

	/*
	采用
	1 3
	0 2
	顶点顺序来绘制每一个面
	*/

	GMModel* m = new GMModel();
	m->setPrimitiveTopologyMode(GMTopologyMode::Triangles);
	m->setType(GMModelType::Model3D);
	GMMesh* face = new GMMesh(m);

	//Front
	{
		GMVertex V0 = {
			{ vertices[0][0], vertices[0][1], vertices[0][2] }, //position
			{ 0, 0, -1.f }, //normal
			{ 0, 1 }, //texcoord
		},
		V1 = {
			{ vertices[1][0], vertices[1][1], vertices[1][2] }, //position
			{ 0, 0, -1.f }, //normal
			{ 0, 0 }, //texcoord
		},
		V2 = {
			{ vertices[2][0], vertices[2][1], vertices[2][2] }, //position
			{ 0, 0, -1.f }, //normal
			{ 1, 1 }, //texcoord
		},
		V3 = {
			{ vertices[3][0], vertices[3][1], vertices[3][2] }, //position
			{ 0, 0, -1.f }, //normal
			{ 1, 0 }, //texcoord
		};
		face->vertex(V0);
		face->vertex(V1);
		face->vertex(V2);
		face->vertex(V1);
		face->vertex(V3);
		face->vertex(V2);
	}

	//Back
	{
		GMVertex V0 = {
			{ vertices[6][0], vertices[6][1], vertices[6][2] }, //position
			{ 0, 0, 1.f }, //normal
			{ 0, 1 }, //texcoord
		},
		V1 = {
			{ vertices[7][0], vertices[7][1], vertices[7][2] }, //position
			{ 0, 0, 1.f }, //normal
			{ 0, 0 }, //texcoord
		},
		V2 = {
			{ vertices[4][0], vertices[4][1], vertices[4][2] }, //position
			{ 0, 0, 1.f }, //normal
			{ 1, 1 }, //texcoord
		},
		V3 = {
			{ vertices[5][0], vertices[5][1], vertices[5][2] }, //position
			{ 0, 0, 1.f }, //normal
			{ 1, 0 }, //texcoord
		};
		face->vertex(V0);
		face->vertex(V1);
		face->vertex(V2);
		face->vertex(V1);
		face->vertex(V3);
		face->vertex(V2);
	}

	//Left
	{
		GMVertex V0 = {
			{ vertices[4][0], vertices[4][1], vertices[4][2] }, //position
			{ -1.f, 0, 0 }, //normal
			{ 0, 1 }, //texcoord
		},
		V1 = {
			{ vertices[5][0], vertices[5][1], vertices[5][2] }, //position
			{ -1.f, 0, 0 }, //normal
			{ 0, 0 }, //texcoord
		},
		V2 = {
			{ vertices[0][0], vertices[0][1], vertices[0][2] }, //position
			{ -1.f, 0, 0 }, //normal
			{ 1, 1 }, //texcoord
		},
		V3 = {
			{ vertices[1][0], vertices[1][1], vertices[1][2] }, //position
			{ -1.f, 0, 0 }, //normal
			{ 1, 0 }, //texcoord
		};
		face->vertex(V0);
		face->vertex(V1);
		face->vertex(V2);
		face->vertex(V1);
		face->vertex(V3);
		face->vertex(V2);
	}

	//Right
	{
		GMVertex V0 = {
			{ vertices[2][0], vertices[2][1], vertices[2][2] }, //position
			{ 1.f, 0, 0 }, //normal
			{ 0, 1 }, //texcoord
		},
		V1 = {
			{ vertices[3][0], vertices[3][1], vertices[3][2] }, //position
			{ 1.f, 0, 0 }, //normal
			{ 0, 0 }, //texcoord
		},
		V2 = {
			{ vertices[6][0], vertices[6][1], vertices[6][2] }, //position
			{ 1.f, 0, 0 }, //normal
			{ 1, 1 }, //texcoord
		},
		V3 = {
			{ vertices[7][0], vertices[7][1], vertices[7][2] }, //position
			{ 1.f, 0, 0 }, //normal
			{ 1, 0 }, //texcoord
		};
		face->vertex(V0);
		face->vertex(V1);
		face->vertex(V2);
		face->vertex(V1);
		face->vertex(V3);
		face->vertex(V2);
	}

	//Bottom
	{
		GMVertex V0 = {
			{ vertices[4][0], vertices[4][1], vertices[4][2] }, //position
			{ 0, -1.f, 0 }, //normal
			{ 0, 1 }, //texcoord
		},
		V1 = {
			{ vertices[0][0], vertices[0][1], vertices[0][2] }, //position
			{ 0, -1.f, 0 }, //normal
			{ 0, 0 }, //texcoord
		},
		V2 = {
			{ vertices[6][0], vertices[6][1], vertices[6][2] }, //position
			{ 0, -1.f, 0 }, //normal
			{ 1, 1 }, //texcoord
		},
		V3 = {
			{ vertices[2][0], vertices[2][1], vertices[2][2] }, //position
			{ 0, -1.f, 0 }, //normal
			{ 1, 0 }, //texcoord
		};
		face->vertex(V0);
		face->vertex(V1);
		face->vertex(V2);
		face->vertex(V1);
		face->vertex(V3);
		face->vertex(V2);
	}

	//Top
	{
		GMVertex V0 = {
			{ vertices[1][0], vertices[1][1], vertices[1][2] }, //position
			{ 0, 1.f, 0 }, //normal
			{ 0, 1 }, //texcoord
		},
		V1 = {
			{ vertices[5][0], vertices[5][1], vertices[5][2] }, //position
			{ 0, 1.f, 0 }, //normal
			{ 0, 0 }, //texcoord
		},
		V2 = {
			{ vertices[3][0], vertices[3][1], vertices[3][2] }, //position
			{ 0, 1.f, 0 }, //normal
			{ 1, 1 }, //texcoord
		},
		V3 = {
			{ vertices[7][0], vertices[7][1], vertices[7][2] }, //position
			{ 0, 1.f, 0 }, //normal
			{ 1, 0 }, //texcoord
		};
		face->vertex(V0);
		face->vertex(V1);
		face->vertex(V2);
		face->vertex(V1);
		face->vertex(V3);
		face->vertex(V2);
	}

	model = GMAsset(GMAssetType::Model, m);
}

void GMPrimitiveCreator::createQuadrangle(const GMVec2& halfExtents, GMfloat z, REF GMModelAsset& model)
{
	constexpr GMfloat texcoord[4][2] =
	{
		{ 0, 1 },
		{ 0, 0 },
		{ 1, 1 },
		{ 1, 0 },
	};

	// 排列方式：
	// 2 4
	// 1 3
	const GMfloat x = halfExtents.getX(), y = halfExtents.getY();
	const GMfloat s_vertices[4][3] = {
		{ -x, -y, z },
		{ -x, y, z },
		{ x, -y, z },
		{ x, y, z },
	};

	GMModel* m = new GMModel();
	m->setPrimitiveTopologyMode(GMTopologyMode::TriangleStrip);
	GMMesh* mesh = new GMMesh(m);

	GMVertex V1 = {
		{ s_vertices[0][0], s_vertices[0][1], s_vertices[0][2] }, //position
		{ 0, -1.f, 0 }, //normal
		{ texcoord[0][0], texcoord[0][1] }, //texcoord
	},
	V2 = {
		{ s_vertices[1][0], s_vertices[1][1], s_vertices[1][2] }, //position
		{ 0, -1.f, 0 }, //normal
		{ texcoord[1][0], texcoord[1][1] }, //texcoord
	},
	V3 = {
		{ s_vertices[2][0], s_vertices[2][1], s_vertices[2][2] }, //position
		{ 0, -1.f, 0 }, //normal
		{ texcoord[2][0], texcoord[2][1] }, //texcoord
	},
	V4 = {
		{ s_vertices[3][0], s_vertices[3][1], s_vertices[3][2] }, //position
		{ 0, -1.f, 0 }, //normal
		{ texcoord[3][0], texcoord[3][1] }, //texcoord
	};
	mesh->vertex(V1);
	mesh->vertex(V2);
	mesh->vertex(V3);
	mesh->vertex(V4);

	model = GMAsset(GMAssetType::Model, m);
}

void GMPrimitiveCreator::createSphere(GMfloat radius, GMint segmentsX, GMint segmentsY, REF GMModelAsset& model)
{
	GM_ASSERT(radius > 0 && segmentsX > 1 && segmentsY > 1);
	GMModel* m = new GMModel();
	m->setDrawMode(GMModelDrawMode::Index);
	m->setPrimitiveTopologyMode(GMTopologyMode::TriangleStrip);
	GMMesh* mesh = new GMMesh(m);

	for (GMint y = 0; y <= segmentsY; ++y)
	{
		for (GMint x = 0; x <= segmentsX; ++x)
		{
			GMfloat xSegment = (GMfloat)x / segmentsX;
			GMfloat ySegment = (GMfloat)y / segmentsY;
			GMfloat xPos = Cos(xSegment * 2.0f * PI) * Sin(ySegment * PI);
			GMfloat yPos = Cos(ySegment * PI);
			GMfloat zPos = -Sin(xSegment * 2.0f * PI) * Sin(ySegment * PI);
			GMVertex v = {
				{ xPos * radius, yPos * radius, zPos * radius },
				{ xPos, yPos, zPos },
				{ xSegment, ySegment }
			};
			mesh->vertex(v);
		}
	}

	bool oddRow = false;
	for (GMint y = 0; y < segmentsY; ++y)
	{
		if (!oddRow)
		{
			for (GMint x = 0; x <= segmentsX; ++x)
			{
				mesh->index(y       * (segmentsX + 1) + x);
				mesh->index((y + 1) * (segmentsX + 1) + x);
			}
		}
		else
		{
			for (GMint x = segmentsX; x >= 0; --x)
			{
				mesh->index((y + 1) * (segmentsX + 1) + x);
				mesh->index(y       * (segmentsX + 1) + x);
			}
		}
		oddRow = !oddRow;
	}

	model = GMAsset(GMAssetType::Model, m);
}

void GMPrimitiveCreator::createTerrain(
	const GMImage& img,
	GMfloat x,
	GMfloat z,
	GMfloat length,
	GMfloat width,
	GMfloat scaling,
	GMsize_t sliceM,
	GMsize_t sliceN,
	REF GMModelAsset& model
)
{
	// 从灰度图创建地形
	const GMfloat x_start = x;
	const GMfloat z_start = z;
	GMVertices vertices;
	vertices.reserve( (sliceM + 1) * (sliceN + 1) );

	const GMfloat dx = length / sliceM; // 2D横向
	const GMfloat dz = width / sliceN; // 2D纵向
	const GMfloat du = 1.f / sliceM;
	const GMfloat dv = 1.f / sliceN;

	// 先计算顶点坐标
	GMfloat y = 0;
	GMfloat u = 0, v = 0;
	GMint x_image = 0, y_image = 0;
	
	for (GMsize_t i = 0; i < sliceN + 1; ++i)
	{
		for (GMsize_t j = 0; j < sliceM + 1; ++j)
		{
			x_image = (x - x_start) * img.getWidth() / length;
			y = scaling * img.getData().mip[0].data[ (x_image + y_image * img.getWidth()) * img.getData().channels ] / 0xFF;

			GMVertex vert = { { x, y, z }, { 0, 0, 0 }, { u, v } };
			vertices.push_back(std::move(vert));
			x += dx;
			u += du;
		}

		z += dz;
		v += dv;
		y_image = (z - z_start) * img.getHeight() / width;
		x = x_start;
	}

	// 再计算法线，一个顶点的法线等于相邻三角形平均值
#define getAdjVert(x, y, pos) getAdjacentVertex(vertices, x, y, sliceM, pos)
	GM_ASSERT(vertices.size() == (sliceM + 1) * (sliceN + 1));
	for (GMsize_t i = 0; i < sliceN + 1; ++i)
	{
		for (GMsize_t j = 0; j < sliceM + 1; ++j)
		{
			// 角上的顶点，直接计算Normal，边上的顶点取2个三角形Normal平均值，中间的则取4个
			// 四角的情况
			if (i == 0 && j == 0) //左下角
			{
				GMVec3 normal = calculateNormal(getVertex(j, i), getAdjVert(j, i, Up), getAdjVert(j, i, Right));
				getVertex(0, 0).normals = { normal.getX(), normal.getY(), normal.getZ() };
			}
			else if (i == 0 && j == sliceM) //右下角
			{
				GMVec3 normal = calculateNormal(getVertex(j, i), getAdjVert(j, i, Left), getAdjVert(j, i, Up));
				getVertex(0, 0).normals = { normal.getX(), normal.getY(), normal.getZ() };
			}
			else if (i == sliceN && j == 0)
			{
				GMVec3 normal = calculateNormal(getVertex(j, i), getAdjVert(j, i, Right), getAdjVert(j, i, Down));
				getVertex(0, 0).normals = { normal.getX(), normal.getY(), normal.getZ() };
			}
			else if (i == sliceN && j == sliceM)
			{
				GMVec3 normal = calculateNormal(getVertex(j, i), getAdjVert(j, i, Down), getAdjVert(j, i, Left));
				getVertex(0, 0).normals = { normal.getX(), normal.getY(), normal.getZ() };
			}
			// 四条边的情况
			else if (i == 0)
			{
				GMVec3 normal0 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Left), getAdjVert(j, i, Up));
				GMVec3 normal1 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Up), getAdjVert(j, i, Right));
				GMVec3 normal = Normalize((normal0 + normal1) / 2);
				getVertex(j, i).normals = { normal.getX(), normal.getY(), normal.getZ() };
			}
			else if (j == 0)
			{
				GMVec3 normal0 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Up), getAdjVert(j, i, Right));
				GMVec3 normal1 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Right), getAdjVert(j, i, Down));
				GMVec3 normal = Normalize((normal0 + normal1) / 2);
				getVertex(j, i).normals = { normal.getX(), normal.getY(), normal.getZ() };
			}
			else if (j == sliceM)
			{
				GMVec3 normal0 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Left), getAdjVert(j, i, Up));
				GMVec3 normal1 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Down), getAdjVert(j, i, Left));
				GMVec3 normal = Normalize((normal0 + normal1) / 2);
				getVertex(j, i).normals = { normal.getX(), normal.getY(), normal.getZ() };
			}
			else if (i == sliceN)
			{
				GMVec3 normal0 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Right), getAdjVert(j, i, Down));
				GMVec3 normal1 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Down), getAdjVert(j, i, Left));
				GMVec3 normal = Normalize((normal0 + normal1) / 2);
				getVertex(j, i).normals = { normal.getX(), normal.getY(), normal.getZ() };
			}
			// 中央
			else
			{
				GMVec3 normal0 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Left), getAdjVert(j, i, Up));
				GMVec3 normal1 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Up), getAdjVert(j, i, Right));
				GMVec3 normal2 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Right), getAdjVert(j, i, Down));
				GMVec3 normal3 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Down), getAdjVert(j, i, Left));
				GMVec3 normal = Normalize((normal0 + normal1 + normal2 + normal3) / 4);
				getVertex(j, i).normals = { normal.getX(), normal.getY(), normal.getZ() };
			}
		}
	}
#undef getAdjVert

	GMModel* m = new GMModel();
	GMMesh* mesh = new GMMesh(m);
	mesh->swap(vertices);

	// 顶点数据创建完毕
	// 接下来创建indices
	m->setDrawMode(GMModelDrawMode::Index);
	m->setPrimitiveTopologyMode(GMTopologyMode::Triangles);
	for (GMsize_t i = 0; i < sliceN; ++i)
	{
		for (GMsize_t j = 0; j < sliceM; ++j)
		{
			mesh->index(getIndex(j, i));
			mesh->index(getIndex(j, i + 1));
			mesh->index(getIndex(j + 1, i + 1));

			mesh->index(getIndex(j, i));
			mesh->index(getIndex(j + 1, i + 1));
			mesh->index(getIndex(j + 1, i));
		}
	}

	model = GMAsset(GMAssetType::Model, m);
}

void GMPrimitiveCreator::createQuad(GMfloat extents[3], GMfloat position[3], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback, GMModelType type, GMCreateAnchor anchor, GMfloat (*customUV)[12])
{
	static constexpr GMfloat v_anchor_center[] = {
		-1, 1, 0,
		-1, -1, 0,
		1, -1, 0,
		1, 1, 0,
	};

	const GMfloat(*_customUV)[12] = customUV ? customUV : &v_anchor_center;
	const GMfloat(&uvArr)[12] = *_customUV;

	static constexpr GMfloat v_anchor_top_left[] = {
		0, 0, 0,
		0, -2, 0,
		2, -2, 0,
		2, 0, 0,
	};

	const GMfloat(&v)[12] = (anchor == TopLeft) ? v_anchor_top_left : v_anchor_center;

	static constexpr GMint indices[] = {
		0, 3, 1,
		2, 1, 3,
	};

	GMModel* model = new GMModel();

	// 实体
	GMfloat t[12];
	for (GMint i = 0; i < 12; ++i)
	{
		t[i] = extents[i % 3] * v[i] + position[i % 3];
	}

	{
		model->setType(type);
		model->setPrimitiveTopologyMode(GMTopologyMode::Triangles);

		GMMesh* body = new GMMesh(model);
		GMFloat4 f4_vertex, f4_normal, f4_uv;
		for (GMint i = 0; i < 2; ++i)
		{
			for (GMint j = 0; j < 3; ++j) // j表示面的一个顶点
			{
				GMint idx = i * 3 + j; //顶点的开始
				GMint idx_next = i * 3 + (j + 1) % 3;
				GMint idx_prev = i * 3 + (j + 2) % 3;
				GMVec2 uv(uvArr[indices[idx] * 3], uvArr[indices[idx] * 3 + 1]);
				GMVec3 vertex(t[indices[idx] * 3], t[indices[idx] * 3 + 1], t[indices[idx] * 3 + 2]);
				GMVec3 vertex_prev(t[indices[idx_prev] * 3], t[indices[idx_prev] * 3 + 1], t[indices[idx_prev] * 3 + 2]),
					vertex_next(t[indices[idx_next] * 3], t[indices[idx_next] * 3 + 1], t[indices[idx_next] * 3 + 2]);
				GMVec3 normal = Cross(vertex - vertex_prev, vertex_next - vertex);
				normal = FastNormalize(normal);

				vertex.loadFloat4(f4_vertex);
				normal.loadFloat4(f4_normal);
				uv.loadFloat4(f4_uv);

				GMVertex v = {
					{ f4_vertex[0], f4_vertex[1], f4_vertex[2] },
					{ f4_normal[0], f4_normal[1], f4_normal[2] },
				};

				if (customUV)
				{
					v.texcoords[0] = f4_uv[0];
					v.texcoords[1] = 1 - f4_uv[1];
				}
				else
				{
					v.texcoords[0] = (f4_uv[0] + 1) / 2;
					v.texcoords[1] = 1 - (f4_uv[1] + 1) / 2;
				}

				v.color[0] = v.color[1] = v.color[2] = 1.f;
				body->vertex(v);
			}
		}
		if (shaderCallback)
			shaderCallback->onCreateShader(model->getShader());
	}

	*obj = model;
}

void GMPrimitiveCreator::createQuad3D(GMfloat extents[3], GMfloat position[12], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback, GMModelType type, GMfloat(*customUV)[8])
{
	static constexpr GMfloat defaultUV[] = {
		-1, 1,
		-1, -1,
		1, -1,
		1, 1,
	};

	const GMfloat(*_pos)[12] = (GMfloat(*)[12])(position);
	const GMfloat(*_uv)[8] = customUV ? customUV : (GMfloat(*)[8])defaultUV;
	const GMfloat(&uvArr)[8] = *_uv;
	const GMfloat(&v)[12] = *_pos;

	static constexpr GMint indices[] = {
		0, 3, 1,
		2, 1, 3,
	};

	GMModel* model = new GMModel();

	// 实体
	GMfloat t[12];
	for (GMint i = 0; i < 12; i++)
	{
		t[i] = extents[i % 3] * v[i];
	}

	{
		model->setType(type);
		GMMesh* body = new GMMesh(model);
		model->setPrimitiveTopologyMode(GMTopologyMode::TriangleStrip);

		GMFloat4 f4_vertex, f4_normal, f4_uv;
		for (GMint i = 0; i < 2; i++)
		{
			for (GMint j = 0; j < 3; j++) // j表示面的一个顶点
			{
				GMint idx = i * 3 + j; //顶点的开始
				GMint idx_next = i * 3 + (j + 1) % 3;
				GMint idx_prev = i * 3 + (j + 2) % 3;
				GMVec2 uv(uvArr[indices[idx] * 2], uvArr[indices[idx] * 2 + 1]);
				GMVec3 vertex(t[indices[idx] * 3], t[indices[idx] * 3 + 1], t[indices[idx] * 3 + 2]);
				GMVec3 vertex_prev(t[indices[idx_prev] * 3], t[indices[idx_prev] * 3 + 1], t[indices[idx_prev] * 3 + 2]),
					vertex_next(t[indices[idx_next] * 3], t[indices[idx_next] * 3 + 1], t[indices[idx_next] * 3 + 2]);
				GMVec3 normal = Cross(vertex - vertex_prev, vertex_next - vertex);
				normal = FastNormalize(normal);

				vertex.loadFloat4(f4_vertex);
				normal.loadFloat4(f4_normal);
				uv.loadFloat4(f4_uv);

				GMVertex v = {
					{ f4_vertex[0], f4_vertex[1], f4_vertex[2] },
					{ f4_normal[0], f4_normal[1], f4_normal[2] }
				};

				if (customUV)
				{
					v.texcoords[0] = f4_uv[0];
					v.texcoords[1] = 1 - f4_uv[1];
				}
				else
				{
					v.texcoords[0] = (f4_uv[0] + 1) / 2;
					v.texcoords[1] = 1 - (f4_uv[1] + 1) / 2;
				}
				v.color[0] = v.color[1] = v.color[2] = v.color[3] = 1.f;
				body->vertex(v);
			}
		}
		if (shaderCallback)
			shaderCallback->onCreateShader(model->getShader());
	}

	*obj = model;
}

GMTextureAsset GMToolUtil::createTexture(const IRenderContext* context, const GMString& filename, REF GMint* width, REF GMint* height)
{
	GMImage* img = nullptr;
	GMBuffer buf;
	GM.getGamePackageManager()->readFile(GMPackageIndex::Textures, filename, &buf);
	GMImageReader::load(buf.buffer, buf.size, &img);
	GM_ASSERT(img);

	GMTextureAsset texture;
	GM.getFactory()->createTexture(context, img, texture);
	GM_ASSERT(!texture.isEmpty());

	if (width)
		*width = img->getWidth();

	if (height)
		*height = img->getHeight();

	GM_delete(img);
	return std::move(texture);
}

void GMToolUtil::createTextureFromFullPath(const IRenderContext* context, const GMString& filename, REF GMTextureAsset& texture, REF GMint* width, REF GMint* height)
{
	GMImage* img = nullptr;
	GMBuffer buf;
	GM.getGamePackageManager()->readFileFromPath(filename, &buf);
	GMImageReader::load(buf.buffer, buf.size, &img);
	GM_ASSERT(img);

	GM.getFactory()->createTexture(context, img, texture);
	GM_ASSERT(!texture.isEmpty());

	if (width)
		*width = img->getWidth();

	if (height)
		*height = img->getHeight();

	GM_delete(img);
}

void GMToolUtil::addTextureToShader(GMShader& shader, GMAsset texture, GMTextureType type)
{
	GM_ASSERT(texture.getType() == GMAssetType::Texture);
	auto& frames = shader.getTextureList().getTextureSampler(type);
	frames.addFrame(texture);
}

bool GMToolUtil::createPBRTextures(
	const IRenderContext* context,
	const GMString& albedoPath,
	const GMString& metallicPath,
	const GMString& roughnessPath,
	const GMString& aoPath,
	const GMString& normalPath,
	REF GMTextureAsset& albedoTexture,
	REF GMTextureAsset& metallicRoughnessAoTexture,
	REF GMTextureAsset& normalTexture
)
{
	bool useWhiteAO = aoPath.isEmpty();

	albedoTexture = GMToolUtil::createTexture(context, albedoPath);
	normalTexture = GMToolUtil::createTexture(context, normalPath);
	if (albedoTexture.isEmpty() || normalPath.isEmpty())
		return false;

	GMBuffer metallicBuf, roughnessBuf, aoBuf;
	GMImage* metallicImg = nullptr, *roughnessImg = nullptr, *aoImg = nullptr;

	GM.getGamePackageManager()->readFile(GMPackageIndex::Textures, metallicPath, &metallicBuf);
	if (!GMImageReader::load(metallicBuf.buffer, metallicBuf.size, &metallicImg))
		goto Failed;

	GM.getGamePackageManager()->readFile(GMPackageIndex::Textures, roughnessPath, &roughnessBuf);
	if (!GMImageReader::load(roughnessBuf.buffer, roughnessBuf.size, &roughnessImg))
		goto Failed;

	if (!useWhiteAO)
	{
		GM.getGamePackageManager()->readFile(GMPackageIndex::Textures, aoPath, &aoBuf);
		if (!GMImageReader::load(aoBuf.buffer, aoBuf.size, &aoImg))
			goto Failed;
	}

	GMint mw = metallicImg->getWidth(), mh = metallicImg->getHeight();
	GMint rw = roughnessImg->getWidth(), rh = roughnessImg->getHeight();
	GMint aow = aoImg ? aoImg->getWidth() : mw, aoh = aoImg ? aoImg->getHeight() : mh;

	if (mw == rw && rw == aow && mh == rh && rh == aoh)
	{
		GMint metallicStep = metallicImg->getData().channels;
		GMint roughnessStep = roughnessImg->getData().channels;

		GMImage combinedImage;
		GMImage::Data& data = combinedImage.getData();
		data.target = GMImageTarget::Texture2D;
		data.mipLevels = 1;
		data.format = GMImageFormat::RGBA;
		data.internalFormat = GMImageInternalFormat::RGBA8;
		data.type = GMImageDataType::UnsignedByte;
		data.mip[0].height = metallicImg->getHeight();
		data.mip[0].width = metallicImg->getWidth();

		GMint sz = data.mip[0].width * data.mip[0].height * 4;
		data.mip[0].data = new GMbyte[sz];
		GMbyte* metallicPtr = metallicImg->getData().mip[0].data;
		GMbyte* roughnessPtr = roughnessImg->getData().mip[0].data;

		GMint aoStep = aoImg ? aoImg->getData().channels : 0;
		GMbyte* aoPtr = aoImg ? aoImg->getData().mip[0].data : nullptr;
		for (GMint p = 0; p < sz; p+=4, metallicPtr+=metallicStep, roughnessPtr+=roughnessStep, aoPtr+=aoStep)
		{
			data.mip[0].data[p] = *metallicPtr;
			data.mip[0].data[p + 1] = *roughnessPtr;
			data.mip[0].data[p + 2] = useWhiteAO ? 0xFF : *aoPtr;
			data.mip[0].data[p + 3] = 0xFF;
		}

		GM.getFactory()->createTexture(context, &combinedImage, metallicRoughnessAoTexture);
		GM_delete(metallicImg);
		GM_delete(roughnessImg);
		GM_delete(aoImg);
		return true;
	}

Failed:
	GM_delete(metallicImg);
	GM_delete(roughnessImg);
	GM_delete(aoImg);
	return false;
}

void GMToolUtil::createCocos2DParticleSystem(const GMString& filename, OUT GMParticleSystem** particleSystem)
{
	if (particleSystem)
	{
		GMBuffer buf;
		GM.getGamePackageManager()->readFile(GMPackageIndex::Particle, filename, &buf);
		buf.convertToStringBuffer();

		*particleSystem = new GMParticleSystem();
		(*particleSystem)->setDescription(GMParticleSystem::createParticleDescriptionFromCocos2DPlist(gm::GMString((const char*)buf.buffer)));
	}
}