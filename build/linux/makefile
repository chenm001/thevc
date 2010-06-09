### enforce 32-bit build : 1=yes, 0=no
M32?= 0
 
export M32

all:
	$(MAKE) -C lib/TLibVideoIO 	MM32=$(M32)
	$(MAKE) -C lib/TLibCommon 	MM32=$(M32)
	$(MAKE) -C lib/TLibDecoder 	MM32=$(M32)
	$(MAKE) -C lib/TLibEncoder 	MM32=$(M32)
	$(MAKE) -C test/TAppDecoder MM32=$(M32)
	$(MAKE) -C test/TAppEncoder MM32=$(M32)

debug:
	$(MAKE) -C lib/TLibVideoIO 	debug	MM32=$(M32)
	$(MAKE) -C lib/TLibCommon 	debug	MM32=$(M32)
	$(MAKE) -C lib/TLibDecoder 	debug	MM32=$(M32)
	$(MAKE) -C lib/TLibEncoder 	debug	MM32=$(M32)
	$(MAKE) -C test/TAppDecoder debug MM32=$(M32)
	$(MAKE) -C test/TAppEncoder debug MM32=$(M32)

release:
	$(MAKE) -C lib/TLibVideoIO 	release MM32=$(M32)
	$(MAKE) -C lib/TLibCommon 	release MM32=$(M32)
	$(MAKE) -C lib/TLibDecoder 	release MM32=$(M32)
	$(MAKE) -C lib/TLibEncoder 	release MM32=$(M32)
	$(MAKE) -C test/TAppDecoder release MM32=$(M32)
	$(MAKE) -C test/TAppEncoder release MM32=$(M32)

clean:
	$(MAKE) -C lib/TLibVideoIO 	clean MM32=$(M32)
	$(MAKE) -C lib/TLibCommon 	clean MM32=$(M32)
	$(MAKE) -C lib/TLibDecoder 	clean MM32=$(M32)
	$(MAKE) -C lib/TLibEncoder 	clean MM32=$(M32)
	$(MAKE) -C test/TAppDecoder clean MM32=$(M32)
	$(MAKE) -C test/TAppEncoder clean MM32=$(M32)