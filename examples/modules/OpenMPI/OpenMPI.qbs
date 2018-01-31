import qbs 1.0

Module
{
	name: 'OpenMPI'

	Depends{name:'cpp'}

	cpp.includePaths: ['/usr/lib/openmpi/include', '/usr/lib/openmpi/include/openmpi']
	
	cpp.libraryPaths: ['/usr/lib/openmpi']
	cpp.dynamicLibraries: ['mpi_cxx', 'mpi', 'dl', 'hwloc']
}
