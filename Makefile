build: clean configure
	@mkdir -p ./.output
	@gcc \
		src/*.c \
		-o ./.output/acqueduct \
		-std=gnu99 \
		-lz

package: build
	@echo "Package unimplemented."

configure:
	@echo ""

clean:
	@rm -rf ./.output

run: build
	@rm -f /tmp/acq
	@mkfifo /tmp/acq

	@tail -f /tmp/acq | ./.output/acqueduct -h "localhost" -p "4004"
