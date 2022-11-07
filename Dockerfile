FROM ubuntu:22.04

RUN apt update && \
	apt install -y gcc-11 python3 python3-pip git make cmake pkg-config \
				   libxft-dev && \
	pip3 install conan

RUN mkdir ctwenty48
WORKDIR /ctwenty48/

#dep mgmt
ADD conan conan
RUN mkdir -p /root/.conan/profiles && \
	cp conan/default-profile /root/.conan/profiles/default

RUN conan create conan/tensorflow
RUN conan create conan/cppflow
RUN conan create conan/csv-parser
RUN conan create conan/nana

RUN mkdir gui_app && mkdir core
ADD cpp/gui_app/conanfile.txt gui_app/conanfile.txt
ADD cpp/core/conanfile.txt core/conanfile.txt
RUN mkdir build && cd build \
	conan install ../gui_app --build=missing && \
	conan install ../core --build=missing

ADD neural-net/models/v1 neural-net/models/v1

#build
ADD cpp/ .
RUN cd build && \
	cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=1 .. && \
	make -j6

CMD build/gui_app/bin/ctwenty48_gui

