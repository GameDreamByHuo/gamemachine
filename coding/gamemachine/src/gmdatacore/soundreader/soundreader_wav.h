﻿#ifndef __SOUNDREADER_WAV_H__
#define __SOUNDREADER_WAV_H__
#include "common.h"
#include "soundreader.h"
BEGIN_NS

class SoundReader_Wav : public GMObject, public ISoundReader
{
public:
	virtual bool load(GamePackageBuffer& buffer, OUT ISoundFile** sf) override;
	virtual bool test(const GamePackageBuffer& buffer) override;
};

END_NS
#endif