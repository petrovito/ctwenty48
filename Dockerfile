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

RUN mkdir gui
ADD gui/conanfile.txt gui/conanfile.txt
RUN mkdir build && cd build \
	conan install ../gui --build=missing

#build
ADD . .
RUN cd build && \
	cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=1 .. && \
	make -j6

CMD build/bin/ctwenty48_gui

