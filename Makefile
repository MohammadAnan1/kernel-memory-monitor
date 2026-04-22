EXTRA_CFLAGS += -Wno-error
obj-m += monitor.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc engine.c -o engine
	gcc workload1.c -o workload1
	gcc workload2.c -o workload2

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f engine workload1 workload2
