//
//  BrightnessManager.h
//
//  Created by Chris Marrin on 3/25/2018
//
//

/*
Copyright (c) 2009-2018 Chris Marrin (chris@marrin.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice, this 
      list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright notice, 
      this list of conditions and the following disclaimer in the documentation 
      and/or other materials provided with the distribution.

    - Neither the name of Marrinator nor the names of its contributors may be 
      used to endorse or promote products derived from this software without 
      specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE.
*/

#pragma once

#include <Ticker.h>

// BrightnessManager
//
// Periodically checks lightSensor port for a voltage and computes a brightness
// from that, with hysteresis. Samples at sampleRate ms and accumulates numSamples.
// Samples are clamped to maxLevels and then normalized to between 0 and numLevels - 1.

class BrightnessManager
{
public:
	BrightnessManager(uint8_t lightSensor, uint16_t maxLevel, uint8_t numLevels, 
					  uint32_t sampleRate = 100, uint8_t numSamples = 5)
		: _lightSensor(lightSensor)
		, _maxLevel(maxLevel)
		, _numLevels(numLevels)
		, _numSamples(numSamples)
	{
		_ticker.attach_ms(sampleRate, compute, this);
	}
	
	uint8_t brightness() const { return _currentBrightness; }
	
	virtual void callback(uint8_t brightness) = 0;
	
private:
	static void compute(BrightnessManager* self) { self->computeBrightness(); }

	void computeBrightness()
	{
		uint16_t ambientLightLevel = analogRead(_lightSensor);
		
		uint32_t brightnessLevel = ambientLightLevel;
		if (brightnessLevel > _maxLevel) {
			brightnessLevel = _maxLevel;
		}
	
		// Make brightness level between 1 and _numLevels
		brightnessLevel = (brightnessLevel * _numLevels + (_maxLevel / 2)) / _maxLevel;
		if (brightnessLevel >= _numLevels) {
			brightnessLevel = _numLevels - 1;
		}
		
		_brightnessAccumulator += brightnessLevel;		

		if (++_brightnessSampleCount >= _numSamples) {
			_brightnessSampleCount = 0;
			uint32_t brightness = (_brightnessAccumulator + (_numSamples / 2)) / _numSamples;
			_brightnessAccumulator = 0;

			if (brightness != _currentBrightness) {
				_currentBrightness = brightness;
				callback(_currentBrightness);
			}
		}
	}

	uint8_t _currentBrightness = 255;
	uint32_t _brightnessAccumulator = 0;
	uint8_t _brightnessSampleCount = 0;
	Ticker _ticker;
	uint8_t _lightSensor;
	uint16_t _maxLevel;
	uint8_t _numLevels;
	uint8_t _numSamples;
};
