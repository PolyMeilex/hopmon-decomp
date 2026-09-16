#!/usr/bin/env python3
"""Extract Hopmon.exe's binary Win32 resources into res/"""

import argparse
import struct
import pefile

from pathlib import Path

LANG_ENGLISH_US = 0x0409

RT_BITMAP = 2
RT_ICON = 3
RT_DIALOG = 5
RT_ACCELERATOR = 9
RT_RCDATA = 10
RT_GROUP_ICON = 14
RT_MANIFEST = 24


def iter_resources(pe):
    for type_entry in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        type_id = str(type_entry.name) if type_entry.name is not None else type_entry.struct.Id
        for res_entry in type_entry.directory.entries:
            res_id = str(res_entry.name) if res_entry.name is not None else res_entry.struct.Id
            for lang_entry in res_entry.directory.entries:
                data = pe.get_data(lang_entry.data.struct.OffsetToData, lang_entry.data.struct.Size)
                yield type_id, res_id, lang_entry.struct.Id, data


def make_bmp(dib):
    (header_size,) = struct.unpack_from("<I", dib, 0)
    (bit_count,) = struct.unpack_from("<H", dib, 14)
    (colors_used,) = struct.unpack_from("<I", dib, 32)
    palette_colors = colors_used or (1 << bit_count if bit_count <= 8 else 0)
    pixel_offset = 14 + header_size + palette_colors * 4
    file_size = 14 + len(dib)
    header = struct.pack("<HIHHI", 0x4D42, file_size, 0, 0, pixel_offset)
    return header + dib


def make_ico(group_data, icon_images):
    (count,) = struct.unpack_from("<H", group_data, 4)
    dir_entries = []
    images = []
    data_offset = 6 + count * 16

    for i in range(count):
        entry_offset = 6 + i * 14
        w, h, colors, res, planes, bitcount, size, res_id = struct.unpack_from(
            "<BBBBHHIH", group_data, entry_offset
        )
        image = icon_images[res_id]
        dir_entries.append(
            struct.pack("<BBBBHHII", w, h, colors, res, planes, bitcount, size, data_offset)
        )
        images.append(image)
        data_offset += len(image)

    return struct.pack("<HHH", 0, 1, count) + b"".join(dir_entries) + b"".join(images)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--exe", required=True)
    parser.add_argument("--out", default="./res")
    args = parser.parse_args()

    pe = pefile.PE(args.exe)
    out = Path(args.out)
    icon_group = None
    icon_images = {}

    for type_id, res_id, lang, data in iter_resources(pe):
        assert lang == LANG_ENGLISH_US, f"unexpected language {lang:#x}"

        if type_id == RT_BITMAP:
            (out / "textures" / f"{res_id}.bmp").write_bytes(make_bmp(data))
        elif type_id == "WAVE":
            (out / "sounds" / f"{res_id}.wav").write_bytes(data)
        elif type_id == RT_RCDATA:
            if data.startswith(b"xof "):
                (out / "models" / f"{res_id}.x").write_bytes(data)
            else:
                (out / "maps" / f"{res_id}.txt").write_bytes(data)
        elif type_id == RT_ICON:
            icon_images[res_id] = data
        elif type_id == RT_GROUP_ICON:
            icon_group = data
        elif type_id == RT_MANIFEST:
            (out / "manifest" / "Hopmon.exe.manifest").write_bytes(data)
        elif type_id in (RT_DIALOG, RT_ACCELERATOR, "AFX_DIALOG_LAYOUT"):
            pass  # text-only, already committed verbatim in res/hopmon.rc
        else:
            raise ValueError(f"unhandled resource type {type_id!r} id {res_id!r}")

    assert icon_group is not None
    (out / "icons" / "Hopmon.ico").write_bytes(make_ico(icon_group, icon_images))


if __name__ == "__main__":
    main()
