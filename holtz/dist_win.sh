#!/bin/bash
cd ..;
DIR=holtz;
zip -r $(DIR)/holtz-win32.zip $(DIR)/holtz.exe $(DIR)/AUTHORS $(DIR)/COPYING $(DIR)/skins/*.zip $(DIR)/help/ \
	$(DIR)/sounds/*.wav $(DIR)/locale/*/LC_MESSAGES/*.mo;
cd -;
