install: ./include/ConfigurationRepository.h
.PHONY: install

./include/ConfigurationRepository.h:
	./add-dependencies
	./dockb

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~
