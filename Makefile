.PHONY: upload build install

PIO = C:\Users\borja.garcia\.platformio\penv\Scripts\platformio.exe

upload: .pio\build\d1_mini\firmware.bin
	python -m esptool --port COM21 write_flash -fm dout -fs 4MB 0x0 .pio\build\d1_mini\firmware.bin
build:
	$(PIO) run
install:
	curl -o install.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
	python install.py
	rm install.py
	python -m pip install esptool