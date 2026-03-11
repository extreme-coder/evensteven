#!/usr/bin/env python3
"""Generate synthetic demo WAV files for EvenSteven."""

import struct
import math
import os
import random

SAMPLE_RATE = 44100
DURATION = 30  # seconds
NUM_SAMPLES = SAMPLE_RATE * DURATION


def write_wav(path, samples, sample_rate=SAMPLE_RATE, channels=2):
    """Write 16-bit PCM WAV file."""
    num_frames = len(samples) // channels
    data_size = num_frames * channels * 2
    file_size = 36 + data_size

    with open(path, 'wb') as f:
        f.write(b'RIFF')
        f.write(struct.pack('<I', file_size))
        f.write(b'WAVE')
        f.write(b'fmt ')
        f.write(struct.pack('<I', 16))  # fmt chunk size
        f.write(struct.pack('<H', 1))   # PCM
        f.write(struct.pack('<H', channels))
        f.write(struct.pack('<I', sample_rate))
        f.write(struct.pack('<I', sample_rate * channels * 2))  # byte rate
        f.write(struct.pack('<H', channels * 2))  # block align
        f.write(struct.pack('<H', 16))  # bits per sample
        f.write(b'data')
        f.write(struct.pack('<I', data_size))

        for s in samples:
            clamped = max(-1.0, min(1.0, s))
            f.write(struct.pack('<h', int(clamped * 32767)))


def pink_noise_sample():
    """Simple pink noise approximation using Voss-McCartney algorithm."""
    return (random.random() * 2 - 1) * 0.3


def generate_balanced_mix():
    """Song 1: Balanced mix with vocal-range tones and accompaniment."""
    samples = []
    for i in range(NUM_SAMPLES):
        t = i / SAMPLE_RATE

        # Vocal simulation: sine tones in vocal range (300-3400 Hz)
        vocal = 0.0
        vocal += 0.15 * math.sin(2 * math.pi * 440 * t)  # A4
        vocal += 0.10 * math.sin(2 * math.pi * 880 * t)  # A5
        vocal += 0.05 * math.sin(2 * math.pi * 1200 * t)

        # Add envelope to make it more musical
        env = 0.5 + 0.5 * math.sin(2 * math.pi * 0.5 * t)
        vocal *= env

        # Accompaniment: bass + mid range
        accomp = 0.0
        accomp += 0.12 * math.sin(2 * math.pi * 110 * t)  # bass
        accomp += 0.08 * math.sin(2 * math.pi * 220 * t)
        accomp += 0.05 * math.sin(2 * math.pi * 330 * t)
        accomp += pink_noise_sample() * 0.05

        mix = vocal + accomp
        # Stereo: slightly different L/R
        samples.append(mix * 0.9)  # Left
        samples.append(mix * 1.0)  # Right

    return samples


def generate_masked_vocals():
    """Song 2: Vocals masked by loud accompaniment (+6 dB)."""
    samples = []
    for i in range(NUM_SAMPLES):
        t = i / SAMPLE_RATE

        # Quiet vocals
        vocal = 0.0
        vocal += 0.08 * math.sin(2 * math.pi * 440 * t)
        vocal += 0.05 * math.sin(2 * math.pi * 880 * t)

        env = 0.5 + 0.5 * math.sin(2 * math.pi * 0.3 * t)
        vocal *= env

        # Loud accompaniment (+6 dB = 2x amplitude)
        accomp = 0.0
        accomp += 0.25 * math.sin(2 * math.pi * 110 * t)
        accomp += 0.20 * math.sin(2 * math.pi * 220 * t)
        accomp += 0.15 * math.sin(2 * math.pi * 165 * t)
        accomp += 0.10 * math.sin(2 * math.pi * 330 * t)
        accomp += pink_noise_sample() * 0.10

        mix = vocal + accomp
        samples.append(mix * 0.9)
        samples.append(mix * 1.0)

    return samples


def generate_loud_song():
    """Song 3: Everything louder (+4 dB vs song 1)."""
    samples = []
    gain = 1.585  # +4 dB

    for i in range(NUM_SAMPLES):
        t = i / SAMPLE_RATE

        vocal = 0.0
        vocal += 0.15 * math.sin(2 * math.pi * 440 * t)
        vocal += 0.10 * math.sin(2 * math.pi * 880 * t)
        vocal += 0.05 * math.sin(2 * math.pi * 1200 * t)

        env = 0.5 + 0.5 * math.sin(2 * math.pi * 0.5 * t)
        vocal *= env

        accomp = 0.0
        accomp += 0.12 * math.sin(2 * math.pi * 110 * t)
        accomp += 0.08 * math.sin(2 * math.pi * 220 * t)
        accomp += 0.05 * math.sin(2 * math.pi * 330 * t)
        accomp += pink_noise_sample() * 0.05

        mix = (vocal + accomp) * gain
        samples.append(mix * 0.9)
        samples.append(mix * 1.0)

    return samples


def main():
    output_dir = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'examples', 'audio')
    os.makedirs(output_dir, exist_ok=True)

    random.seed(42)

    print("Generating demo_song_1.wav (balanced mix)...")
    write_wav(os.path.join(output_dir, 'demo_song_1.wav'), generate_balanced_mix())

    print("Generating demo_song_2.wav (vocals masked)...")
    random.seed(43)
    write_wav(os.path.join(output_dir, 'demo_song_2.wav'), generate_masked_vocals())

    print("Generating demo_song_3.wav (loud song)...")
    random.seed(44)
    write_wav(os.path.join(output_dir, 'demo_song_3.wav'), generate_loud_song())

    print(f"Demo audio files written to {output_dir}")


if __name__ == '__main__':
    main()
