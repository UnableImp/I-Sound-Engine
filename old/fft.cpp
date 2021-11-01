
constexpr double PI = 3.1415926535897932,
        DELTA = 0.00003051757; // 2^-15
typedef std::complex<double>* complexList;

std::array<std::array<std::complex<double>,1024>,11> fourierTable = []
{
    std::array<std::array<std::complex<double>,1024>,11> arr = {};
    for(int s = 1; s <= 11; ++s)
    {
        int m = 1 << s;
        std::complex<double> wm(std::cos((2.0f * PI) / m ), -std::sin((2.0f * PI) / m));
        std::complex<double> w = 1;
        for(int j = 0; j < (m/2.0f); ++j)
        {
            arr[s-1][j] = w;
            w *= wm;

        }
        arr[10][1023] = w;
    }
    return arr;
}();

std::array<int, 2048> bitTable = []
{
    std::array<int, 2048> table;
    int size = std::log2(8 - 1) + 1;
    int sum = 0;


    for(unsigned i = 1; i < 8; ++i)
    {
        int shift = size - 1;
        while(sum & 1 << shift)
        {
            sum ^= 1 << shift;
            --shift;
        }
        sum |= 1 << shift;

        table[sum] = i;
    }

    return table;
}();

static void bitReverseOrder(double* list, complexList& newList, unsigned n)
{
    int size = std::log2(n - 1) + 1;
    int sum = 0;
    newList[0] = list[0];

    for(unsigned i = 1; i < n; ++i)
    {
        int shift = size - 1;
        while(sum & 1 << shift)
        {
            sum ^= 1 << shift;
            --shift;
        }
        sum |= 1 << shift;

        newList[sum] = list[i];
    }
}


template<typename Type>
static void fft2(double* list, int n, complexList& rList)
{
    //bitReverseOrder(list, rList, n);

    for(int s = 1; s <= std::log2(n); ++s)
    {
        int m = 1 << s;

        for(int k = 0; k < n; k += m)
        {
            std::complex<Type> w = 1;

            for(int j = 0; j < (m/2.0f); ++j)
            {
                std::complex<Type> u = list[bitTable[k + j]];
                __m256d uv = {u.real(), u.imag(), u.real(), u.imag()};

                __m256d wv = {w.real(), w.imag(), w.imag(), w.real()};

                std::complex<Type> t = fourierTable[s-1][j] * list[bitTable[k + j + m/2]];
                __m256d tv = {t.real(), t.imag(), -t.real(), -t.imag()};

                __m256d r = _mm256_add_pd(tv, uv);

                rList[k + j] = *reinterpret_cast<std::complex<double>*>(&r);
                rList[k + j + m/2] = *reinterpret_cast<std::complex<double>*>((&r)+1);

//                rList[k + j] = u + t;
//                rList[k + j + m/2] = u - t;

                w = fourierTable[s-1][j];
            }
        }
    }
}

static void bitReverseOrder(complexList const& list, complexList& newList, unsigned n)
{
    int size = std::log2(n - 1) + 1;
    int sum = 0;
    newList[0] = list[0];

    for(unsigned i = 1; i < n; ++i)
    {
        int shift = size - 1;
        while(sum & 1 << shift)
        {
            sum ^= 1 << shift;
            --shift;
        }
        sum |= 1 << shift;

        newList[sum] = list[i];
    }
}

static void Offt2(complexList const& list, int n, complexList& rList)
{

    bitReverseOrder(list, rList, n);

    for(int s = 1; s <= std::log2(n); ++s)
    {
        int m = 1 << s;
        std::complex<double> wm(std::cos((2.0f * PI) / m ), -std::sin((2.0f * PI) / m));

        for(int k = 0; k < n; k += m)
        {
            std::complex<double> w = 1;

            for(int j = 0; j < (m/2.0); ++j)
            {
                std::complex<double> t = w * rList[k + j + m/2];

                std::complex<double> u = rList[k + j];

                rList[k + j] = u + t;
                rList[k + j + m/2] = u - t;

                w *= wm;
            }
        }
    }
}

static void MyFFT2048(benchmark::State& state)
{
    const int size = 2048;
    complexList complex = new std::complex<double>[size];
    double data[2048] = {};
    Frame<float> songData[2048];
    Get2048Samples(songData);

    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }

    for(auto _ : state)
    {
        fft2<double>(data, size, complex);
    }
}
BENCHMARK(MyFFT2048);

static void MyFFT1024(benchmark::State& state)
{
    const int size = 1024;
    complexList complex = new std::complex<double>[size];
    double data[2048] = {};
    Frame<float> songData[2048];
    Get2048Samples(songData);
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }
    for(auto _ : state)
    {
        fft2<double>(data, size, complex);
        fft2<double>(data, size, complex);
    }
}
BENCHMARK(MyFFT1024);

TEST(FFT, MyFFT512)
{
const int size = 8;
complexList complex = new std::complex<double>[size]();
complexList complex1o = new std::complex<double>[size]();
complexList complex2o = new std::complex<double>[size]();

double data[8] = {};
//    Frame<float> songData[2048];
//    Get2048Samples(songData);
//    for(int i = 0; i < 2048; ++i)
//    {
//        data[i] = songData[i].leftChannel;
//    }
//    for(int i = 0; i < 2048; ++i)
//    {
//        data[i] = songData[i].leftChannel;
//    }

data[0] = 1;
data[1] = 0.5;
data[2] = -0.5;
data[3] = -1;
data[4] = -0.2;
data[5] = 0.2;
complex[0] = 1;
complex[1] = 0.5;
complex[2] = -0.5;
complex[3] = -1;
complex[4] = -0.2;
complex[5] = 0.2;

fft2<double>(data, size, complex1o);
Offt2(complex, size, complex2o);

for(int i = 0; i < size; ++i)
{
std::cout << complex1o[i] << " " << complex2o[i] << std::endl;
}

}

static void MyFFT512(benchmark::State& state)
{
    const int size = 512;
    complexList complex = new std::complex<double>[size];
    double data[2048] = {};
    Frame<float> songData[2048];
    Get2048Samples(songData);
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }

    for(auto _ : state)
    {
        fft2<double>(data, size, complex);
        fft2<double>(data, size, complex);
        fft2<double>(data, size, complex);
        fft2<double>(data, size, complex);
    }
}
BENCHMARK(MyFFT512);


