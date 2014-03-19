#include <stdlib.h>
#include <stdio.h>
#include "catcierge_timer.h"
#include "minunit.h"
#include "catcierge_test_helpers.h"

char *run_tests()
{
	catcierge_timer_t t;
	double val;

	catcierge_timer_reset(&t);
	catcierge_timer_start(&t);
	catcierge_timer_set(&t, 5.0);

	catcierge_test_STATUS("Wait 5 seconds for timer...");
	mu_assert("Expected timer to be active", catcierge_timer_isactive(&t));

	sleep(5);
	val = catcierge_timer_get(&t);

	mu_assert("Expected 5 seconds to elapse", (val >= 5.0));
	mu_assert("Expected timer to have values less than 6.0", 
			(val < 6.0));
	mu_assert("Expected timer to have timed out", 
			catcierge_timer_has_timed_out(&t));

	catcierge_test_STATUS("Reset timer...");
	catcierge_timer_reset(&t);

	mu_assert("Expected timer to be 0.0", 
			catcierge_timer_get(&t) == 0.0);

	return NULL;
}

int TEST_catcierge_timer(int argc, char **argv)
{
	char *e = NULL;

	catcierge_test_HEADLINE("TEST_catcierge_timer");

	if ((e = run_tests())) catcierge_test_FAILURE(e);
	else catcierge_test_SUCCESS("");
	
	return 0;
}
