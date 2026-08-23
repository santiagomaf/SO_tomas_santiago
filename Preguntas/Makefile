# Makefile raíz: delega en el Makefile de cada subcarpeta de ejercicio.
# Uso: make <nombre-carpeta>   -> compila esa carpeta
#      make run-<nombre-carpeta> -> compila y corre esa carpeta
#      make all                -> compila todas las carpetas
#      make clean              -> limpia los binarios de todas las carpetas

SUBDIRS = fork-pipe-exec ejercicio-guiado padovan-threads primos-stats

.PHONY: all clean $(SUBDIRS) $(addprefix run-,$(SUBDIRS))

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

$(addprefix run-,$(SUBDIRS)):
	$(MAKE) -C $(patsubst run-%,%,$@) run

clean:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d clean; done
