from distutils.core import setup, Extension
import sysconfig

def main():
    CFLAGS = ['-g', '-Wall', '-std=c99', '-fopenmp', '-mavx', '-mfma', '-pthread', '-O3']
    LDFLAGS = ['-fopenmp']
    # Use the setup function we imported and set up the modules.
    # You may find this reference helpful: https://docs.python.org/3.6/extending/building.html
    # TODO: YOUR CODE HERE
    numc_module = Extension(
                    'numc', # Module name
                    extra_compile_args=CFLAGS,  # Add CFLAGS to compilation
                    extra_link_args=LDFLAGS,  # Add LDFLAGS to linking
                    sources = ['numc.c', 'matrix.c'])   # # Source file(s)

    setup (name = 'numc',
           version = '1.0',
           description = 'numc module',
           author = 'zjy',
           ext_modules = [numc_module])

if __name__ == "__main__":
    main()
