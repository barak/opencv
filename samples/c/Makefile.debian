OBJECTS = demhist convexhull delaunay distrans drawing edge facedetect ffilldemo fitellipse kalman kmeans laplace minarea morphology motempl pyramid_segmentation squares

all: $(OBJECTS)

clean:
	rm -f $(OBJECTS)

demhist: demhist.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

convexhull: convexhull.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

delaunay: delaunay.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

distrans: distrans.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

drawing: drawing.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

edge: edge.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

facedetect: facedetect.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv` # cvaux !!!

ffilldemo: ffilldemo.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

fitellipse: fitellipse.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

kalman: kalman.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

kmeans: kmeans.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

laplace: laplace.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

minarea: minarea.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

morphology: morphology.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

motempl: motempl.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

pyramid_segmentation: pyramid_segmentation.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

squares: squares.c
	$(CXX) $^ -o $@ `pkg-config --cflags --libs opencv`

