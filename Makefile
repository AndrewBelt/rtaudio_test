
include ../arch.mk


CXXFLAGS += -I../dep/include
LDFLAGS += -L../dep/lib -lrtaudio

TARGET = rtaudio_test


all: $(TARGET)

run: $(TARGET)
ifeq ($(ARCH), lin)
	LD_LIBRARY_PATH=../dep/lib ./$<
endif
ifeq ($(ARCH), mac)
	DYLD_FALLBACK_LIBRARY_PATH=../dep/lib ./$<
endif
ifeq ($(ARCH), win)
	env PATH=../dep/bin:/mingw64/bin ./$<
endif


SOURCES += rtaudio_test.cpp

include ../compile.mk


clean:
	rm -rfv build $(TARGET)
