TEMPLATE = app
QMAKE_LFLAGS += "-Wl,--hash-style=gnu -Wl,--as-needed"

DESTDIR = $${OUT_PWD}/../package/root/bin

copy_ringtone.target = $$DESTDIR/Phone.wav
copy_ringtone.depends = $$_PRO_FILE_PWD_/Phone.wav
copy_ringtone.commands = $(COPY_FILE) \"$$replace(copy_ringtone.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_ringtone.target, /, $$QMAKE_DIR_SEP)\"
QMAKE_EXTRA_TARGETS += copy_ringtone
PRE_TARGETDEPS += $$copy_ringtone.target
