#include <vector>

template <class T> class List: private std::vector<T> {
	typedef std::vector<T> parent;
public:
	int count () const {
		return parent::size ();
	}
	void append (const T& element) {
		parent::push_back (element);
	}
	T& get (int i) {
		return parent::operator [] (i);
	}
	const T& get (int i) const {
		return parent::operator [] (i);
	}
	T& operator [] (int i) {
		return parent::operator [] (i);
	}
	const T& operator [] (int i) const {
		return parent::operator [] (i);
	}
};
