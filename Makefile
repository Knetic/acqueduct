build: clean
	@mkdir -p ./.output
	@gcc \
		src/*.c \
		-o ./.output/acqueduct \
		-std=gnu99 \
		-lz

package: build

ifeq ($(shell which fpm), )
	@echo "FPM is not installed, no packages will be made."
	@echo "https://github.com/jordansissel/fpm"
	@exit 1
endif

ifeq ($(ACQ_VERSION), )

	@echo "No 'ACQ_VERSION' was specified."
	@echo "Export a 'ACQ_VERSION' environment variable to perform a package"
	@exit 1
endif

	@rm -f ./*.deb
	@rm -f ./*.rpm

	fpm \
		--log error \
		-s dir \
		-t deb \
		-d zlib1g-dev \
		-d zlib1g \
		-v $(ACQ_VERSION) \
		-n acqueduct \
		./.output/acqueduct=/usr/local/bin/acqueduct

	@mv ./*.deb ./.output

	fpm \
		--log error \
		-s dir \
		-t rpm \
		-d zlib \
		-v $(ACQ_VERSION) \
		-n acqueduct \
		./.output/acqueduct=/usr/local/bin/acqueduct

	@mv ./*.rpm ./.output/

clean:
	@rm -rf ./.output

run: build
	@rm -f /tmp/acq
	@mkfifo /tmp/acq

	@tail -f /tmp/acq | ./.output/acqueduct -h "localhost" -p "4004"

lint: clean
	@gcc \
		src/*.c \
		-Wall \
		-o /dev/null \
		-std=gnu99 \
		-lz
