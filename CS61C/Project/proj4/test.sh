make
cd unittests
python -m unittest unittests.TestAdd.test_func -v
python -m unittest unittests.TestAdd.test_small_add -v
python -m unittest unittests.TestSub.test_func -v
python -m unittest unittests.TestSub.test_small_sub -v
python -m unittest unittests.TestAbs.test_func -v
python -m unittest unittests.TestAbs.test_small_abs -v
python -m unittest unittests.TestNeg.test_func -v
python -m unittest unittests.TestNeg.test_small_neg -v
python -m unittest unittests.TestMul.test_func -v
python -m unittest unittests.TestMul.test_small_mul -v
python -m unittest unittests.TestPow.test_func -v
python -m unittest unittests.TestPow.test_small_pow -v
python -m unittest unittests.TestGet -v
python -m unittest unittests.TestSet -v
python -m unittest unittests.TestShape -v
python -m unittest unittests.TestSubscript -v
python -m unittest unittests.TestSetSubscript -v
cd ..