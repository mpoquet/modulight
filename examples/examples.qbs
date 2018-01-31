import qbs 1.0

Project
{
	CppApplication
	{
		name: '1_helloModule'
		files: ['1_hello/hello_module.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '1_helloApp'
		files: ['1_hello/hello_app.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '2_tic'
		files: ['2_tictac/tic.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '2_tac'
		files: ['2_tictac/tac.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '2_tictac'
		files: ['2_tictac/tictac.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '3_tic'
		files: ['3_tictac_with_parameters/tic.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '3_tac'
		files: ['3_tictac_with_parameters/tac.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '3_tictac'
		files: ['3_tictac_with_parameters/tictac.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '4_dynamic_master'
		files: ['4_dynamism/dynamic_master.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '4_dynamic_tic'
		files: ['4_dynamism/tic.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '4_dynamic_tac'
		files: ['4_dynamism/tac.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '4_dynamic_tictac'
		files: ['4_dynamism/app.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '5_parallel_module'
		files: ['5_parallel_with_duplication/parallel_module.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '5_parallel_app'
		files: ['5_parallel_with_duplication/parallel_app.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '5_generator'
		files: ['5_parallel_with_duplication/generator.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '6_parallel_module'
		files: ['6_parallel_without_duplication/parallel_module.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '6_parallel_app'
		files: ['6_parallel_without_duplication/parallel_app.cpp']

		Depends { name: 'modulight' }
	}

	CppApplication
	{
		name: '6_generator'
		files: ['6_parallel_without_duplication/generator.cpp']

		Depends { name: 'modulight' }
	}
}