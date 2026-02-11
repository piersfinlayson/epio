DOXYGEN_AWESOME_CSS = doxygen-awesome/doxygen-awesome.css
DOXYGEN_AWESOME_URL = https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/main/doxygen-awesome.css

$(DOXYGEN_AWESOME_CSS):
	mkdir -p doxygen-awesome
	curl -o $(DOXYGEN_AWESOME_CSS) $(DOXYGEN_AWESOME_URL)

docs: clean-docs $(DOXYGEN_AWESOME_CSS)
	doxygen Doxyfile

clean-docs:
	rm -rf docs