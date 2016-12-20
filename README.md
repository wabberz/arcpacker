# arcpacker
MT-Engine ARC Un/Repacker Ver. 0.9.0 BETA, (c)2016 Uwabami
Original Python code by FrozenFish24, ported to C++ and extended by Uwabami.

The code is not very neat at the moment and I plan to clean it up and encapsulate it soon. There will also be a DLL with exposed API.

## Use
```	arc [flags] [file(s)/folder(s)]```

## Flags
```	-c {0-9}	force zlib compression level
				 0 - no compression
				 9 - maximum compression
				-1 - will use manifest/default
	-dd		pack Dual Destinies compatible ARC
	-f		overwrite ARC files without asking
	-h		show this information and quit
	-o [file]	define archive/folder name for output
	-q		no output except errors (will override verbose flags)
	-v		be verbose
	-vv		be very verbose
	-vvv		be extremely verbose (show debug messages)```

## Notes

Usually, the compression level of each file will be applied from the manifest.
If there's no manifest, it will default to 6 - but the games will not accept an
ARC with a different file order, so keep this in mind. If you define a level
outside the range of 0-9, the flag will be ignored.

Input will be auto-detected. If you supply a folder, it will be compressed to an
ARC file. An ARC file will be unpacked to a folder. You can pack/unpack
multiple folders/files at the same time.
