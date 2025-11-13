#!/usr/bin/env python3
"""
Ultrasound Dummy Data Generator
Generates .ultra files with realistic ultrasound-like imagery
"""

import struct
import os
import numpy as np
from datetime import datetime
import zlib

# Constants
MAGIC_ID = 0x554C5452  # "ULTR"
WIDTH = 128
HEIGHT = 128
DEPTH = 1
BYTES_PER_VOXEL = 1
HEADER_SIZE = 32

def calculate_crc32(data):
    """Calculate CRC32 checksum for data validation"""
    return zlib.crc32(data) & 0xFFFFFFFF

def create_header(frame_number, timestamp):
    """
    Create a 32-byte header for .ultra file

    Header format:
    - Magic ID (4 bytes): 0x554C5452 ("ULTR")
    - Width (2 bytes): 128
    - Height (2 bytes): 128
    - Depth (2 bytes): 1
    - BytesPerVoxel (2 bytes): 1
    - FrameNumber (4 bytes): sequential
    - Timestamp (8 bytes): microseconds
    - Reserved (4 bytes): for future use
    - CRC (4 bytes): checksum
    """
    header = struct.pack(
        '<IHHHHIQII',  # little-endian format
        MAGIC_ID,
        WIDTH,
        HEIGHT,
        DEPTH,
        BYTES_PER_VOXEL,
        frame_number,
        timestamp,
        0,  # Reserved
        0   # CRC placeholder, will be calculated
    )
    return header

def generate_ultrasound_frame(frame_number, total_frames):
    """
    Generate realistic ultrasound-like image with speckle noise

    Ultrasound characteristics:
    - Grayscale (0-255)
    - Speckle noise pattern
    - Gradual intensity changes to simulate tissue
    - Some variation between frames to simulate motion
    """
    # Create base intensity pattern (simulate tissue layers)
    y_coords = np.linspace(0, 1, HEIGHT)
    x_coords = np.linspace(0, 1, WIDTH)

    # Create depth-dependent intensity (darker at depth)
    depth_pattern = np.outer(1 - 0.3 * y_coords, np.ones(WIDTH))

    # Add some anatomical-like structures
    center_x, center_y = WIDTH // 2, HEIGHT // 2
    y_grid, x_grid = np.ogrid[:HEIGHT, :WIDTH]

    # Circular structure (simulating organ or tissue boundary)
    phase = 2 * np.pi * frame_number / total_frames
    offset_x = int(5 * np.sin(phase))
    offset_y = int(3 * np.cos(phase))

    circle1 = np.sqrt((x_grid - center_x - offset_x)**2 + (y_grid - center_y - offset_y)**2)
    circle1 = np.exp(-circle1**2 / 500) * 0.4

    # Another structure
    circle2 = np.sqrt((x_grid - center_x + 20)**2 + (y_grid - center_y - 15)**2)
    circle2 = np.exp(-circle2**2 / 300) * 0.3

    # Combine patterns
    base_image = depth_pattern + circle1 + circle2

    # Add speckle noise (characteristic of ultrasound)
    # Speckle is multiplicative noise
    speckle = np.random.rayleigh(0.1, (HEIGHT, WIDTH))
    ultrasound_image = base_image * (1 + speckle)

    # Add some Gaussian noise
    gaussian_noise = np.random.normal(0, 0.05, (HEIGHT, WIDTH))
    ultrasound_image += gaussian_noise

    # Normalize to 0-255 range
    ultrasound_image = np.clip(ultrasound_image, 0, 1)
    ultrasound_image = (ultrasound_image * 255).astype(np.uint8)

    return ultrasound_image

def save_ultra_file(filename, frame_number, frame_data):
    """Save frame data with header to .ultra file"""
    # Get timestamp in microseconds
    timestamp = int(datetime.now().timestamp() * 1_000_000)

    # Create header (without CRC)
    header = create_header(frame_number, timestamp)

    # Combine header and data
    full_data = header + frame_data.tobytes()

    # Calculate CRC for the entire data
    crc = calculate_crc32(full_data[28:])  # CRC from after the CRC field

    # Update header with CRC
    header_with_crc = struct.pack(
        '<IHHHHIQII',
        MAGIC_ID,
        WIDTH,
        HEIGHT,
        DEPTH,
        BYTES_PER_VOXEL,
        frame_number,
        timestamp,
        0,  # Reserved
        crc
    )

    # Write to file
    with open(filename, 'wb') as f:
        f.write(header_with_crc)
        f.write(frame_data.tobytes())

    print(f"Generated: {filename} (Frame {frame_number}, CRC: 0x{crc:08X})")

def generate_ultra_files(output_dir, num_files=10):
    """Generate multiple .ultra files simulating a sequence"""
    print(f"Generating {num_files} ultrasound files...")
    print(f"Format: {WIDTH}x{HEIGHT} grayscale, {BYTES_PER_VOXEL} byte per pixel")
    print(f"Output directory: {output_dir}\n")

    os.makedirs(output_dir, exist_ok=True)

    for i in range(num_files):
        frame_data = generate_ultrasound_frame(i, num_files)
        filename = os.path.join(output_dir, f"frame_{i:04d}.ultra")
        save_ultra_file(filename, i, frame_data)

    print(f"\n✓ Successfully generated {num_files} files")

    # Calculate total size
    total_size = num_files * (HEADER_SIZE + WIDTH * HEIGHT * BYTES_PER_VOXEL)
    print(f"Total size: {total_size / 1024:.2f} KB")

def verify_ultra_file(filename):
    """Verify the integrity of an .ultra file"""
    print(f"\nVerifying: {filename}")

    with open(filename, 'rb') as f:
        # Read header
        header_data = f.read(HEADER_SIZE)
        magic, width, height, depth, bpv, frame_num, timestamp, reserved, crc = struct.unpack(
            '<IHHHHIQII', header_data
        )

        print(f"  Magic: 0x{magic:08X} ({'ULTR' if magic == MAGIC_ID else 'INVALID'})")
        print(f"  Dimensions: {width}x{height}x{depth}")
        print(f"  Frame: {frame_num}, Timestamp: {timestamp}")
        print(f"  CRC: 0x{crc:08X}")

        # Read and verify data
        frame_data = f.read()
        expected_size = width * height * depth * bpv

        if len(frame_data) == expected_size:
            print(f"  ✓ Data size correct: {len(frame_data)} bytes")
        else:
            print(f"  ✗ Data size mismatch: {len(frame_data)} vs {expected_size}")

if __name__ == "__main__":
    # Generate files in current directory
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # Generate 15 files for a good demo sequence
    generate_ultra_files(script_dir, num_files=15)

    # Verify the first file
    verify_ultra_file(os.path.join(script_dir, "frame_0000.ultra"))

    print("\n" + "="*50)
    print("File format documentation:")
    print("="*50)
    print(f"Magic ID: 0x554C5452 ('ULTR')")
    print(f"Image size: {WIDTH}x{HEIGHT} pixels")
    print(f"Bit depth: {BYTES_PER_VOXEL * 8} bits (grayscale)")
    print(f"Header size: {HEADER_SIZE} bytes")
    print(f"Frame size: {WIDTH * HEIGHT * BYTES_PER_VOXEL} bytes")
    print(f"Total file size: {HEADER_SIZE + WIDTH * HEIGHT * BYTES_PER_VOXEL} bytes")
    print("="*50)
