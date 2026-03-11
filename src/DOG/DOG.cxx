
#include "DOGCLP.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRecursiveGaussianImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkCastImageFilter.h"

int main(int argc, char * argv[])
{
  PARSE_ARGS;

  if (sigma1 <= 0.0 || sigma2 <= 0.0)
  {
    std::cerr << "ERROR: sigma1 and sigma2 must both be > 0." << std::endl;
    return EXIT_FAILURE;
  }
  if (sigma1 == sigma2)
  {
    std::cerr << "ERROR: sigma1 and sigma2 must be different." << std::endl;
    return EXIT_FAILURE;
  }

  constexpr unsigned int Dimension = 3;
  using InputPixelType  = short;
  using RealPixelType   = float;
  using OutputPixelType = unsigned char;
  using InputImageType  = itk::Image<InputPixelType,  Dimension>;
  using RealImageType   = itk::Image<RealPixelType,   Dimension>;
  using OutputImageType = itk::Image<OutputPixelType, Dimension>;

  // ---------------------------------------------------------------------------
  // Read input and cast to float
  // ---------------------------------------------------------------------------
  using ReaderType = itk::ImageFileReader<InputImageType>;
  auto reader = ReaderType::New();
  reader->SetFileName(inputVolume);

  using CastToRealType = itk::CastImageFilter<InputImageType, RealImageType>;
  auto castToReal = CastToRealType::New();
  castToReal->SetInput(reader->GetOutput());

  // ---------------------------------------------------------------------------
  // Gaussian chain 1 — sigma1
  // All six filter objects are declared in main scope so they stay alive
  // for the entire duration of the pipeline execution.
  // ---------------------------------------------------------------------------
  using GaussianType = itk::RecursiveGaussianImageFilter<RealImageType, RealImageType>;

  auto g1X = GaussianType::New();
  g1X->SetDirection(0);
  g1X->SetSigma(sigma1);
  g1X->SetNormalizeAcrossScale(true);
  g1X->SetInput(castToReal->GetOutput());

  auto g1Y = GaussianType::New();
  g1Y->SetDirection(1);
  g1Y->SetSigma(sigma1);
  g1Y->SetNormalizeAcrossScale(true);
  g1Y->SetInput(g1X->GetOutput());

  auto g1Z = GaussianType::New();
  g1Z->SetDirection(2);
  g1Z->SetSigma(sigma1);
  g1Z->SetNormalizeAcrossScale(true);
  g1Z->SetInput(g1Y->GetOutput());

  // ---------------------------------------------------------------------------
  // Gaussian chain 2 — sigma2
  // ---------------------------------------------------------------------------
  auto g2X = GaussianType::New();
  g2X->SetDirection(0);
  g2X->SetSigma(sigma2);
  g2X->SetNormalizeAcrossScale(true);
  g2X->SetInput(castToReal->GetOutput());

  auto g2Y = GaussianType::New();
  g2Y->SetDirection(1);
  g2Y->SetSigma(sigma2);
  g2Y->SetNormalizeAcrossScale(true);
  g2Y->SetInput(g2X->GetOutput());

  auto g2Z = GaussianType::New();
  g2Z->SetDirection(2);
  g2Z->SetSigma(sigma2);
  g2Z->SetNormalizeAcrossScale(true);
  g2Z->SetInput(g2Y->GetOutput());

  // ---------------------------------------------------------------------------
  // Subtract: DOG = G_sigma1(f) - G_sigma2(f)
  // ---------------------------------------------------------------------------
  using SubtractType = itk::SubtractImageFilter<RealImageType, RealImageType, RealImageType>;
  auto subtract = SubtractType::New();
  subtract->SetInput1(g1Z->GetOutput());
  subtract->SetInput2(g2Z->GetOutput());

  // ---------------------------------------------------------------------------
  // Rescale to [0, 255] and cast to unsigned char
  // ---------------------------------------------------------------------------
  using RescaleType = itk::RescaleIntensityImageFilter<RealImageType, OutputImageType>;
  auto rescale = RescaleType::New();
  rescale->SetInput(subtract->GetOutput());
  rescale->SetOutputMinimum(0);
  rescale->SetOutputMaximum(255);

  // ---------------------------------------------------------------------------
  // Write output
  // ---------------------------------------------------------------------------
  using WriterType = itk::ImageFileWriter<OutputImageType>;
  auto writer = WriterType::New();
  writer->SetFileName(outputVolume);
  writer->SetInput(rescale->GetOutput());

  try
  {
    writer->Update();
  }
  catch (const itk::ExceptionObject & err)
  {
    std::cerr << "ERROR: " << err << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "DOG complete.\n"
            << "  Input  : " << inputVolume  << "\n"
            << "  Output : " << outputVolume << " (unsigned char, 0-255)\n"
            << "  Sigma1 : " << sigma1 << "\n"
            << "  Sigma2 : " << sigma2 << std::endl;

  return EXIT_SUCCESS;
}