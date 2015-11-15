build: clean configure
	@mkdir -p ./.output
	@gcc src/* -o ./.output/acqueduct

package: build
	@echo "Package unimplemented."

configure:
	@echo ""

clean:
	@rm -rf ./.output

run: build
	@./.output/acqueduct
