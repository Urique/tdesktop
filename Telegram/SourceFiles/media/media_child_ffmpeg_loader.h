/*
This file is part of Telegram Desktop,
the official desktop version of Telegram messaging app, see https://telegram.org

Telegram Desktop is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

It is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

In addition, as a special exception, the copyright holders give permission
to link the code of portions of this program with the OpenSSL library.

Full license: https://github.com/telegramdesktop/tdesktop/blob/master/LICENSE
Copyright (c) 2014-2016 John Preston, https://desktop.telegram.org
*/
#pragma once

#include "media/media_audio_loader.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
} // extern "C"

#include <AL/al.h>

struct VideoSoundData {
	uint64 videoPlayId = 0;
	AVCodecContext *context = nullptr;
	int32 frequency = AudioVoiceMsgFrequency;
	int64 length = 0;
	~VideoSoundData();
};

struct VideoSoundPart {
	AVPacket *packet = nullptr;
	uint64 videoPlayId = 0;
};

class ChildFFMpegLoader : public AudioPlayerLoader {
public:
	ChildFFMpegLoader(std_::unique_ptr<VideoSoundData> &&data);

	bool open(qint64 position = 0) override;

	bool check(const FileLocation &file, const QByteArray &data) override {
		return true;
	}

	int32 format() override {
		return _format;
	}

	int64 duration() override {
		return _parentData->length;
	}

	int32 frequency() override {
		return _parentData->frequency;
	}

	ReadResult readMore(QByteArray &result, int64 &samplesAdded) override;
	void enqueuePackets(QQueue<AVPacket> &packets);

	uint64 playId() const {
		return _parentData->videoPlayId;
	}

	~ChildFFMpegLoader();

private:
	int32 _sampleSize = 2 * sizeof(uint16);
	int32 _format = AL_FORMAT_STEREO16;
	int32 _srcRate = AudioVoiceMsgFrequency;
	int32 _dstRate = AudioVoiceMsgFrequency;
	int32 _maxResampleSamples = 1024;
	uint8_t **_dstSamplesData = nullptr;

	std_::unique_ptr<VideoSoundData> _parentData;
	AVSampleFormat _inputFormat;
	AVFrame *_frame = nullptr;

	SwrContext *_swrContext = nullptr;
	QQueue<AVPacket> _queue;

};