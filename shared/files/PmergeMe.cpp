#include "PmergeMe.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iterator>

void printList(const p_list& list) {
    for (p_list::const_iterator it = list.begin(); it != list.end(); ++it) {
        std::cout << it->value << " ";
    }
    std::cout << std::endl;
}

// Create pairs and drop the smallest of the two into an other list.
// The pair are stored by index, high[0] and low[0] are a pair.
static void	spliceList(p_list& high, p_list& low)
{
	p_list::iterator	list_it = high.begin();
	p_list::iterator	end_list = high.end();
	p_list::iterator	next_it = list_it;
	next_it++;

	if (high.size() % 2 != 0)
		end_list--;

	while (list_it != end_list) {
		p_list::iterator	temp_it = next_it;

		if (temp_it != end_list)
			temp_it++;
		if (list_it->value > next_it->value) {
			next_it->links.push_back(&(*list_it));
			low.splice(low.end(), high, next_it);
		} else {
			list_it->links.push_back(&(*next_it));
			low.splice(low.end(), high, list_it);
		}

		list_it = temp_it;
		next_it = ++temp_it;
	}
}

static void switchNodes(p_list& list, p_list::iterator it1, p_list::iterator it2) {
    if (it1 == it2)
    	return;

    list.splice(it2, list, it1);
    list.splice(it1, list, it2);
}

static void	insert(p_list::iterator& element, p_list& to)
{
	p_list::iterator	it = to.begin();

	while (it != to.end()) {
		if (element->value < it->value) {
			to.insert(it, *element);
			return ;
		}
		it++;
	}
}

// Ford-Johnson Merge-insertion sort using lists.
// This function recursivly sort the array of winners by re-using this function onto itself.
int	pmergeList(p_list& list)
{
	p_list	low_n;

	if (list.size() < 2)
		return (0);
	else if (list.size() == 2) {
		if (list.front().value > list.back().value)
			switchNodes(list, list.begin(), ++list.begin());
		return (0);
	}

	// put the pair's smaller number into an other list, winners are left into the list.
	spliceList(list, low_n);

	//std::cout << "-- -- -- unsorted -- -- --" << std::endl;
	//std::cout << "Winners: "; printList(list);
	//std::cout << "Loosers: "; printList(low_n);

	// continue the recursion on the winner list
	pmergeList(list);

	// sort using binary insertion
	for (p_list::iterator it = low_n.begin(); it != low_n.end(); it++) {
		//std::cout << "inserting: " << it->value << std::endl;
		insert(it, list);
	}

	//std::cout << "--- --- --- sorted --- --- ---" << std::endl;
	//printList(list);
	//printList(low_n);

	return (0);
}

/*
94 82 100
29 80 62

Winners: 94 100
Loosers: 82

setp 1: inserting the unpaired elements

82 94 100

step 2: inserting the smaller ones, end make the array ending on the last linked number as it is bigger.

80 is linked with 82, so we have to do binary insertion with 82 as the iterator of ending, doing that for every
other elements into the small numbers.

*/

