
int Example() {

	// in the "nstd" namespace. 
	// a queue that stores integers called q
	nstd::queue<int> q;

	// similar to std syntax. push 1, 2, 3, 4 to the back
	q.push_back(1);
	q.push_back(2);

	// emplace back too (but without fancy construction because I can't be bothered to do that)
	int &number = q.emplace_back( );
	number = 3;

	for (auto& val : q) {
		printf("%d\n", val);
	}

	q.emplace_back();
	q.back() = 4;

	// pop from the front of the queue.
	// behind the scenes data is kept as close as possible (contiguous) with a floating front pointer
	q.pop();
	// when popping, no guarantees about validity of data (like std::vector). do stuff with the data then pop!
	int& a = q.back();
	q.pop();

	// has iterators. iterators are chunky so use the operator[] syntax if you can
	for (auto& val : q) {
		printf("%d\n", val);
	}

	// will clear the size to zero and call the destructors
	q.clear();
	q.push_back(10);
	q.push_back(15);
	q.push_back(17);

	// more iterator syntax. can do all the dumb algorithm stuff to your modern C++ heart's content
	// pushing/popping/clearing will invalidate iterators
	for (auto it = q.begin(); it != q.end(); ++it) {
		const auto i = *it;
		printf("%d\n", i);
	}

	// showcase with weird type that's too complicated for it's own good
	struct ComplicatedType {
		int a;

		ComplicatedType() { printf("constructor called\n"); }
		ComplicatedType(const ComplicatedType& type) { printf("copy constructor called\n"); }
		ComplicatedType(ComplicatedType&& type) { printf("move constructor called\n"); }

		ComplicatedType& operator=(const ComplicatedType& type) { printf("copy assignment called\n"); return *this; }
		ComplicatedType& operator=(ComplicatedType&& type) { printf("move assignment called\n"); return *this; }

		~ComplicatedType() { printf("destructor called\n"); }
	};

	// a queue with the weird type
	ComplicatedType type;
	nstd::queue<ComplicatedType> type_q;

	// when the queue reallocates as capacity is reached, it will attempt to move those 
	// objects rather than copy them
	type_q.push_back(type);
	type_q.push_back(type);
	type_q.push_back(type);
	// move semantics supported for push back
	type_q.push_back(std::move(type));
	type_q.pop();

	// can also just iterate over the whole thing. this is the most useful
	for (int i = 0; i < type_q.size(); i++) {
		type_q[i].a = 10;
	}

	struct TrivialData {
		int* a;
		char c;
		int index;
		bool is_init;
	};
	nstd::queue_trivial<TrivialData> trivial;

	// we choose when to initialise rather than there being an implicit init
	trivial.emplace_back(); // performant because trivial queue assumes no constructor
	trivial.emplace_back([](TrivialData& data) { data.a = nullptr; data.c = 'a'; data.is_init = true; }); // but we can still have one if we want too

	// same for destruction/deinit
	trivial.pop();
	trivial.pop([](TrivialData& data) { data.a = nullptr; data.c = 'n'; data.is_init = false; });

	// although it's just a long winded way of initialising the data, useful for semi-simple types

	return false;
}
