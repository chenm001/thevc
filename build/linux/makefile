all:
	$(MAKE) -C lib/TLibVideoIO
	$(MAKE) -C lib/TLibCommon
	$(MAKE) -C lib/TLibDecoder
	$(MAKE) -C lib/TLibEncoder
	$(MAKE) -C test/TAppDecoder
	$(MAKE) -C test/TAppEncoder

debug:
	$(MAKE) -C lib/TLibVideoIO debug
	$(MAKE) -C lib/TLibCommon debug
	$(MAKE) -C lib/TLibDecoder debug
	$(MAKE) -C lib/TLibEncoder debug
	$(MAKE) -C test/TAppDecoder debug
	$(MAKE) -C test/TAppEncoder debug

release:
	$(MAKE) -C lib/TLibVideoIO release
	$(MAKE) -C lib/TLibCommon release
	$(MAKE) -C lib/TLibDecoder release
	$(MAKE) -C lib/TLibEncoder release
	$(MAKE) -C test/TAppDecoder release
	$(MAKE) -C test/TAppEncoder release

clean:
	$(MAKE) -C lib/TLibVideoIO clean
	$(MAKE) -C lib/TLibCommon clean
	$(MAKE) -C lib/TLibDecoder clean
	$(MAKE) -C lib/TLibEncoder clean
	$(MAKE) -C test/TAppDecoder clean
	$(MAKE) -C test/TAppEncoder clean
