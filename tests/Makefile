protocol_test: protocol_test.cpp ../src/protocol.cpp ../src/protocol.hpp
	g++ -std=c++23 -I. -I../src/ protocol_test.cpp ../src/protocol.cpp -o protocol_test
fb_ops_test: fb_ops_test.cpp ../src/fb_ops.cpp ../src/fb_ops.hpp
	g++ -std=c++23 -I. -I../src/ fb_ops_test.cpp ../src/fb_ops.cpp -o fb_ops_test

test: fb_ops_test protocol_test
	./fb_ops_test && ./protocol_test
