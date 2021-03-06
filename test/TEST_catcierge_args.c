#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "catcierge_args.h"
#include "minunit.h"
#include "catcierge_test_helpers.h"
#include "catcierge_config.h"

char *perform_config_test(int expected_ret, char *test_cfg, catcierge_args_t *args)
{
	int ret;
	FILE *f;

	#define TEST_CONFIG_PATH "______test.cfg"
	char *argv[] = { "program", "--config", TEST_CONFIG_PATH };
	int argc = sizeof(argv) / sizeof(argv[0]);

	// Create a test config.
	f = fopen(TEST_CONFIG_PATH, "w");
	mu_assert("Failed to open test config", f);

	fwrite(test_cfg, 1, strlen(test_cfg), f);
	fclose(f);

	// Init parser.
	memset(args, 0, sizeof(catcierge_args_t));
	ret = catcierge_args_init(args, "catcierge");
	mu_assert("Failed to init catcierge args", ret == 0);

	ret = catcierge_args_parse(args, argc, argv);
	mu_assert("Parse status not expected", (!!ret) == expected_ret);

	return NULL;
}

char *run_parse_config_tests()
{
	#define ASSERT_CONFIG_START(EXPECT_RET, CONF) 						\
	do 																	\
	{																	\
		catcierge_args_t args;											\
		catcierge_test_STATUS("\nConfig:\n%s", CONF);					\
		mu_assert("Failed config:\n"CONF"\n",							\
			perform_config_test(EXPECT_RET, CONF, &args) == NULL)
	

	#define ASSERT_CONFIG_END()											\
		catcierge_args_destroy(&args);									\
	} while (0)



	ASSERT_CONFIG_START(0,
		"template_matcher=1\n");
	mu_assert("Unexpected matcher", args.matcher_type == MATCHER_TEMPLATE);
	ASSERT_CONFIG_END();

	ASSERT_CONFIG_START(0,
		"   template_matcher    =   1   \n");
	mu_assert("Unexpected matcher", args.matcher_type == MATCHER_TEMPLATE);
	ASSERT_CONFIG_END();

	// This only gives a warning that the options is specified twice.
	ASSERT_CONFIG_START(0,
		"template_matcher=2\n");
	mu_assert("Unexpected matcher", args.matcher_type == MATCHER_TEMPLATE);
	ASSERT_CONFIG_END();

	ASSERT_CONFIG_START(1,
		"template_matcher=\n");
	ASSERT_CONFIG_END();

	ASSERT_CONFIG_START(1,
		"template_matcher=1\n"
		"template_matcher=1\n");
	ASSERT_CONFIG_END();

	ASSERT_CONFIG_START(0,
		"haar=1\n");
	mu_assert("Unexpected matcher", args.matcher_type == MATCHER_HAAR);
	ASSERT_CONFIG_END();

	ASSERT_CONFIG_START(1, "haar=broken\n");
	ASSERT_CONFIG_END();

	ASSERT_CONFIG_START(1,
		"haar=1\n"
		"template=1\n");
	ASSERT_CONFIG_END();

	ASSERT_CONFIG_START(0,
		"haar=1\n"
		"ok_matches_needed=4\n");
	printf("ok_matches_needed = %d\n", args.ok_matches_needed);
	mu_assert("Unexpected ok_matches_needed", args.ok_matches_needed == 4);
	ASSERT_CONFIG_END();

	#ifndef _WIN32
	ASSERT_CONFIG_START(0,
		"haar=1\n"
		"base_time=2016-01-23T22:11:03\n");
	mu_assert("Unexpected base_time", !strcmp(args.base_time, "2016-01-23T22:11:03"));
	ASSERT_CONFIG_END();
	#endif

	ASSERT_CONFIG_START(1,
		"haar=1\n"
		"does=not exist\n"
		"what=is this\n");
	ASSERT_CONFIG_END();

	ASSERT_CONFIG_START(1,
		"haar=1\n"
		"lockout_dummy=2502585298529831909139090259050229328998329823");
	ASSERT_CONFIG_END();


	return NULL;
}

static char *run_catierge_add_options_test()
{
	int ret = 0;
	catcierge_args_t args;
	ret = catcierge_args_init(&args, "catcierge");
	mu_assert("Failed to init catcierge args", ret == 0);

	catcierge_print_settings(&args);

	catcierge_args_destroy(&args);

	return NULL;
}

static char *run_catcierge_parse_test()
{
	char *err = NULL;

	#define PARSE_ARGV_START(expect, argsp, ...)							\
		do 																	\
		{ 																	\
			int i; 															\
			int ret = 0;													\
			catcierge_args_t args;											\
			char *argv[] = { __VA_ARGS__ }; 								\
			int argc = sizeof(argv) / sizeof(argv[0]); 						\
			memset(&args, 0, sizeof(args));									\
			ret = catcierge_args_init(&args, "catcierge");					\
			mu_assert("Failed to init catcierge args", ret == 0);			\
			for (i = 0; i < argc; i++) printf("%s ", argv[i]); 				\
			printf("\n"); 													\
			if ((err = perform_catcierge_args(expect, argsp, argc, argv, &ret))) \
			{																\
				printf("Expected: %d Got: %d\n", expect, ret);				\
				return err;													\
			}

	#define PARSE_ARGV_END() 				\
			catcierge_args_destroy(&args); 	\
		} while (0)

	PARSE_ARGV_START(0, &args, "catcierge", "--haar");
	mu_assert("Expected haar matcher", args.matcher_type == MATCHER_HAAR);
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--show");
	mu_assert("show != 1", args.show == 1);
	PARSE_ARGV_END();

	{
		PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--ok_matches_needed", "2");
		mu_assert("ok_matches_needed != 2", args.ok_matches_needed == 2);
		PARSE_ARGV_END();
		PARSE_ARGV_START(-1, &args, "catcierge", "--haar", "--ok_matches_needed");
		PARSE_ARGV_END();
		PARSE_ARGV_START(-1, &args, "catcierge", "--haar", "--ok_matches_needed", "20");
		PARSE_ARGV_END();
	}

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--no_final_decision");
	mu_assert("Expected no_final_decision == 1", args.no_final_decision == 1);
	PARSE_ARGV_END();

	{
		PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--lockout_method", "3");
		mu_assert("Expected lockout_method == 3", args.lockout_method == OBSTRUCT_OR_TIMER_3);
		PARSE_ARGV_END();

		PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--lockout_method", "2");
		mu_assert("Expected lockout_method == 2", args.lockout_method == OBSTRUCT_THEN_TIMER_2);
		PARSE_ARGV_END();

		PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--lockout_method", "1");
		mu_assert("Expected lockout_method == 1", args.lockout_method == TIMER_ONLY_1);
		PARSE_ARGV_END();
		PARSE_ARGV_START(-1, &args, "catcierge", "--haar", "--lockout_method", "0"); PARSE_ARGV_END();
		PARSE_ARGV_START(-1, &args, "catcierge", "--haar", "--lockout_method", "4"); PARSE_ARGV_END();
		PARSE_ARGV_START(-1, &args, "catcierge", "--haar", "--lockout_method"); PARSE_ARGV_END();
	}

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--save");
	mu_assert("Expected save == 1", args.saveimg == 1);
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--save_obstruct");
	mu_assert("Expected save_obstruct == 1", args.save_obstruct_img == 1);
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--highlight");
	mu_assert("Expected highlight == 1", args.highlight_match == 1);
	PARSE_ARGV_END();

	#ifdef WITH_ZMQ
	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--zmq");
	mu_assert("Expected zmq == 1", args.zmq == 1);
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--zmq_port", "1234");
	mu_assert("Expected zmq_port == 1234", args.zmq_port == 1234);
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--zmq_iface", "eth1");
	printf("zmq_iface = %s\n", args.zmq_iface);
	mu_assert("Expected zmq_iface == eth1",
		args.zmq_iface && !strcmp(args.zmq_iface, "eth1"));
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--zmq_transport", "inproc");
	printf("zmq_transport = %s\n", args.zmq_transport);
	mu_assert("Expected zmq_transport == inproc",
		args.zmq_transport && !strcmp(args.zmq_transport, "inproc"));
	PARSE_ARGV_END();
	#endif // WITH_ZMQ

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--lockout", "5");
	mu_assert("Expected lockout == 5", args.lockout_time == 5);
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--lockout_error_delay", "5");
	mu_assert("Expected lockout_error_delay == 5",
			args.consecutive_lockout_delay == 5);
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--lockout_error", "20");
	mu_assert("Expected lockout_error == 20",
			args.max_consecutive_lockout_count == 20);
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--lockout_dummy");
	mu_assert("Expected lockout_dummy == 1", args.lockout_dummy == 1);
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--matchtime", "5");
	mu_assert("Expected match_time == 1", args.match_time == 5);
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--output", "/some/output/path");
	mu_assert("Expected output_path == /some/output/path",
		args.output_path
		&& !strcmp(args.output_path, "/some/output/path"));
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args,
		"catcierge", "--haar", "--match_output_path", "/some/output/path");
	mu_assert("Expected match_output_path == /some/output/path",
		args.match_output_path
		&& !strcmp(args.match_output_path, "/some/output/path"));
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args,
		"catcierge", "--haar", "--steps_output_path", "/some/output/path");
	mu_assert("Expected steps_output_path == /some/output/path",
		args.steps_output_path
		&& !strcmp(args.steps_output_path, "/some/output/path"));
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args,
		"catcierge", "--haar", "--obstruct_output_path", "/some/output/path");
	mu_assert("Expected obstruct_output_path == /some/output/path",
		args.obstruct_output_path
		&& !strcmp(args.obstruct_output_path, "/some/output/path"));
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args,
		"catcierge", "--haar", "--template_output_path", "/some/output/path");
	mu_assert("Expected template_output_path == /some/output/path",
		args.template_output_path
		&& !strcmp(args.template_output_path, "/some/output/path"));
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args,
		"catcierge", "--haar", "--input", "/path/to/templ%time%.tmpl", "bla");
	mu_assert("Expected inputs[0] == /path/to/templ%time%.tmpl",
		args.inputs[0]
		&& !strcmp(args.inputs[0], "/path/to/templ%time%.tmpl"));
	mu_assert("Expected inputs[1] == bla",
		args.inputs[0]
		&& !strcmp(args.inputs[1], "bla"));
	mu_assert("Expected 2 input count", args.input_count == 2);
	PARSE_ARGV_END();


	#ifdef WITH_RFID

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--rfid_in", "/some/rfid/path");
	mu_assert("Expected rfid_inner_path == /some/rfid/path",
		args.rfid_inner_path
		&& !strcmp(args.rfid_inner_path, "/some/rfid/path"));
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--rfid_out", "/some/rfid/path");
	mu_assert("Expected rfid_outer_path == /some/rfid/path",
		args.rfid_outer_path
		&& !strcmp(args.rfid_outer_path, "/some/rfid/path"));
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--rfid_allowed", "1", "2", "3");
	mu_assert("Unexpected rfid_allowed values",
		args.rfid_allowed[0] && !strcmp(args.rfid_allowed[0], "1") &&
		args.rfid_allowed[1] && !strcmp(args.rfid_allowed[1], "2") &&
		args.rfid_allowed[2] && !strcmp(args.rfid_allowed[2], "3") &&
		(args.rfid_allowed_count == 3));
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--rfid_time", "3.5");
	mu_assert("Expected rfid_lock_time == 3.5",
		(args.rfid_lock_time == 3.5));
	PARSE_ARGV_END();

	#else
	catcierge_test_SKIPPED("Skipping RFID args (not compiled)");
	#endif // WITH_RFID


	#define PARSE_CMD_SETTING(cmd)										\
		PARSE_ARGV_START(0, &args,										\
			"catcierge", "--haar", "--"#cmd, "awesome_cmd.sh");			\
		mu_assert("Expected "#cmd" == awesome_cmd.sh",					\
			args.cmd[0] && !strcmp(args.cmd[0], "awesome_cmd.sh"));		\
		PARSE_ARGV_END()

	#define CATCIERGE_DEFINE_EVENT(ev_enum_name, ev_name, ev_description)	\
		PARSE_CMD_SETTING(ev_name ## _cmd);
	#include "catcierge_events.h"

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--nocolor");
	mu_assert("Expected nocolor == 1", (args.nocolor == 1));
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--noanim");
	mu_assert("Expected noanim == 1", (args.noanim == 1));
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--save_steps");
	mu_assert("Expected save_steps == 1", (args.save_steps == 1));
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--auto_roi");
	mu_assert("Expected auto_roi == 1", (args.auto_roi == 1));
	PARSE_ARGV_END();
	
	PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--min_backlight");
	PARSE_ARGV_END();
	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--min_backlight", "25000");
	mu_assert("Expected auto_roi == 1", (args.min_backlight == 25000));
	PARSE_ARGV_END();

	PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--startup_delay");
	PARSE_ARGV_END();
	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--startup_delay", "5.0");
	mu_assert("Expected startup_delay == 1", (args.startup_delay == 5.0));
	PARSE_ARGV_END();

	PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--roi");
	PARSE_ARGV_END();
	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--roi", "1", "2", "3", "4");
	mu_assert("Expected valid roi",
		(args.roi.x == 1) &&
		(args.roi.y == 2) &&
		(args.roi.width == 3) &&
		(args.roi.height == 4));
	PARSE_ARGV_END();

	PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--chuid");
	PARSE_ARGV_END();
	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--chuid", "arne");
	mu_assert("Expected chuid == arne",
		args.chuid && !strcmp(args.chuid, "arne"));
	PARSE_ARGV_END();

	#ifndef _WIN32
	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--base_time", "2014-10-26T14:00:22");
	catcierge_test_STATUS("%s", args.base_time);
	mu_assert("Expected base_time == 2014-10-26T14:00:22",
		args.base_time && !strcmp(args.base_time, "2014-10-26T14:00:22"));
	PARSE_ARGV_END();
	PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--base_time");
	PARSE_ARGV_END();
	PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--base_time", "notatimestamp");
	PARSE_ARGV_END();
	#endif // _WIN32

	// Template matcher setings.
	{
		PARSE_ARGV_START(0, &args, "catcierge", "--templ",
			"--snout", "/the/snout/path/snout.png");
		mu_assert("Unexpected snout path",
			args.templ.snout_count == 1 &&
			args.templ.snout_paths[0] &&
			!strcmp(args.templ.snout_paths[0], "/the/snout/path/snout.png"));
		PARSE_ARGV_END();

		PARSE_ARGV_START(1, &args, "catcierge", "--templ", "--snout");
		PARSE_ARGV_END();

		PARSE_ARGV_START(0, &args, "catcierge", "--templ", "--match_flipped");
		mu_assert("Expected match_flipped == 1", args.templ.match_flipped == 1);
		PARSE_ARGV_END();

		PARSE_ARGV_START(0, &args, "catcierge", "--templ", "--threshold", "0.7");
		mu_assert("Expected threshold == 0.7", args.templ.match_threshold == 0.7);
		PARSE_ARGV_END();

		PARSE_ARGV_START(1, &args, "catcierge", "--templ", "--threshold");
		PARSE_ARGV_END();
	}

	// Haar matcher settings.
	{
		PARSE_ARGV_START(0, &args, "catcierge", "--haar",
			"--cascade", "/path/to/catcierge.xml");
		mu_assert("Expected cascade == /path/to/catcierge.xml",
			args.haar.cascade &&
			!strcmp(args.haar.cascade, "/path/to/catcierge.xml"));
		PARSE_ARGV_END();
		PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--cascade");
		PARSE_ARGV_END();

		PARSE_ARGV_START(0, &args, "catcierge", "--haar",
			"--min_size", "12x34");
		mu_assert("Expected valid min size",
			(args.haar.min_width == 12) && (args.haar.min_height == 34));
		PARSE_ARGV_END();

		PARSE_ARGV_START(0, &args, "catcierge", "--haar",
			"--min_size", "12x34");
		mu_assert("Expected valid min size",
			(args.haar.min_width == 12) && (args.haar.min_height == 34));
		PARSE_ARGV_END();
		PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--min_size");
		PARSE_ARGV_END();
		PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--min_size", "gaeokea");
		PARSE_ARGV_END();

		PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--prey_steps", "0");
		PARSE_ARGV_END();
		PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--prey_steps", "3");
		PARSE_ARGV_END();
		PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--prey_steps");
		PARSE_ARGV_END();
		PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--prey_steps", "2");
		mu_assert("Expected prey_steps == 2", args.haar.prey_steps == 2);
		PARSE_ARGV_END();

		PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--no_match_is_fail");
		mu_assert("Expected no_match_is_fail == 1",
			args.haar.no_match_is_fail == 1);
		PARSE_ARGV_END();

		PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--equalize_histogram");
		mu_assert("Expected equalize_histogram == 1",
			args.haar.eq_histogram == 1);
		PARSE_ARGV_END();

		PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--in_direction", "left");
		mu_assert("Expected in_direction == LEFT",
			args.haar.in_direction == DIR_LEFT);
		PARSE_ARGV_END();
		PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--in_direction", "right");
		mu_assert("Expected in_direction == RIGHT",
			args.haar.in_direction == DIR_RIGHT);
		PARSE_ARGV_END();
		PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--in_direction", "bla");
		PARSE_ARGV_END();
		PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--in_direction");
		PARSE_ARGV_END();

		PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--prey_method", "adaptive");
		mu_assert("Expected prey_method == PREY_METHOD_ADAPTIVE",
			args.haar.prey_method == PREY_METHOD_ADAPTIVE);
		PARSE_ARGV_END();
		PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--prey_method", "normal");
		mu_assert("Expected prey_method == PREY_METHOD_NORMAL",
			args.haar.prey_method == PREY_METHOD_NORMAL);
		PARSE_ARGV_END();
		PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--prey_method", "bla");
		PARSE_ARGV_END();
		PARSE_ARGV_START(1, &args, "catcierge", "--haar", "--prey_method");
		PARSE_ARGV_END();
	}

	PARSE_ARGV_START(1, &args, "catcierge", "--cmdhelp");
	PARSE_ARGV_END();

	PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--uservar", "abc 123");
	PARSE_ARGV_END();

	#define CATCIERGE_SIGUSR_BEHAVIOR(sigusr_name, sigusr_description)					\
		PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--sigusr1", #sigusr_name);	\
		PARSE_ARGV_END();
	#include "catcierge_sigusr_types.h"

	#define CATCIERGE_SIGUSR_BEHAVIOR(sigusr_name, sigusr_description)					\
		PARSE_ARGV_START(0, &args, "catcierge", "--haar", "--sigusr2", #sigusr_name);	\
		PARSE_ARGV_END();
	#include "catcierge_sigusr_types.h"

	return NULL;
}

int TEST_catcierge_args(int argc, char **argv)
{
	char *e = NULL;
	int ret = 0;

	CATCIERGE_RUN_TEST((e = run_catierge_add_options_test()),
		"Run add options test",
		"Add options test", &ret);

	CATCIERGE_RUN_TEST((e = run_catcierge_parse_test()),
		"Run parse test",
		"Parse test", &ret);

	CATCIERGE_RUN_TEST((e = run_parse_config_tests()),
		"Run parse config test",
		"Parse config test", &ret);

	if (ret)
	{
		catcierge_test_FAILURE("One or more tests failed!");
	}

	return ret;
}
