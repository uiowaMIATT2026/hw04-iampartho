
// Applies a 3D recursive Gaussian low-pass filter to reduce noise.
// The same sigma is applied in X, Y, and Z directions.
// Uses itk::RecursiveGaussianImageFilter chained in three passes.
//
// Usage:
//   GaussianLPF_parthghosh \
//       --inputVolume  <input.nii.gz>  \
//       --outputVolume <output.nii.gz> \
//       --sigma        <sigma>
//

#include "GaussianLPFCLP.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRecursiveGaussianImageFilter.h"
#include "itkCastImageFilter.h"

int main(int argc, char * argv[])
{
  // ---------------------------------------------------------------------------
  // 1.  Parse arguments from XML-generated CLP header.
  //     Variables available after this macro:  inputVolume, outputVolume, sigma
  // ---------------------------------------------------------------------------
  PARSE_ARGS;

  if (sigma <= 0.0)
  {
    std::cerr << "ERROR: sigma must be greater than 0. Got: " << sigma << std::endl;
    return EXIT_FAILURE;
  }

  // ---------------------------------------------------------------------------
  // 2.  Image type definitions.
  //
  //     RecursiveGaussianImageFilter produces floating-point output internally,
  //     so we work in float throughout the pipeline and cast back to short for
  //     the output — this preserves the full smoothed-value precision while
  //     staying compatible with signed-short NIFTI viewers.
  // ---------------------------------------------------------------------------
  constexpr unsigned int Dimension = 3;
  using InputPixelType  = short;
  using RealPixelType   = float;
  using InputImageType  = itk::Image<InputPixelType, Dimension>;
  using RealImageType   = itk::Image<RealPixelType,  Dimension>;

  // ---------------------------------------------------------------------------
  // 3.  Read input image.
  // ---------------------------------------------------------------------------
  using ReaderType = itk::ImageFileReader<InputImageType>;
  auto reader = ReaderType::New();
  reader->SetFileName(inputVolume);

  // Cast input from short → float so the Gaussian filter can work in real space.
  using CastToRealType = itk::CastImageFilter<InputImageType, RealImageType>;
  auto castToReal = CastToRealType::New();
  castToReal->SetInput(reader->GetOutput());

  // ---------------------------------------------------------------------------
  // 4.  Chain three RecursiveGaussianImageFilters — one per axis.
  //
  //     RecursiveGaussianImageFilter implements a computationally efficient
  //     recursive approximation of the Gaussian convolution.  It processes
  //     one direction at a time, so we chain X → Y → Z.
  //     SetDirection(0/1/2) selects X/Y/Z respectively.
  //     SetSigma() takes physical-space units (mm for NIFTI).
  //     SetNormalizeAcrossScale(true) keeps the filter scale-invariant.
  // ---------------------------------------------------------------------------
  using GaussianFilterType = itk::RecursiveGaussianImageFilter<RealImageType, RealImageType>;

  auto gaussianX = GaussianFilterType::New();
  gaussianX->SetDirection(0);           // X axis
  gaussianX->SetSigma(sigma);
  gaussianX->SetNormalizeAcrossScale(true);
  gaussianX->SetInput(castToReal->GetOutput());

  auto gaussianY = GaussianFilterType::New();
  gaussianY->SetDirection(1);           // Y axis
  gaussianY->SetSigma(sigma);
  gaussianY->SetNormalizeAcrossScale(true);
  gaussianY->SetInput(gaussianX->GetOutput());

  auto gaussianZ = GaussianFilterType::New();
  gaussianZ->SetDirection(2);           // Z axis
  gaussianZ->SetSigma(sigma);
  gaussianZ->SetNormalizeAcrossScale(true);
  gaussianZ->SetInput(gaussianY->GetOutput());

  // Cast the float result back to short for the output file.
  using CastToShortType = itk::CastImageFilter<RealImageType, InputImageType>;
  auto castToShort = CastToShortType::New();
  castToShort->SetInput(gaussianZ->GetOutput());

  // ---------------------------------------------------------------------------
  // 5.  Write output image.
  // ---------------------------------------------------------------------------
  using WriterType = itk::ImageFileWriter<InputImageType>;
  auto writer = WriterType::New();
  writer->SetFileName(outputVolume);
  writer->SetInput(castToShort->GetOutput());

  try
  {
    writer->Update();
  }
  catch (const itk::ExceptionObject & err)
  {
    std::cerr << "ERROR: " << err << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "GaussianLPF complete.\n"
            << "  Input  : " << inputVolume  << "\n"
            << "  Output : " << outputVolume << "\n"
            << "  Sigma  : " << sigma << " (applied in X, Y, Z)" << std::endl;

  return EXIT_SUCCESS;
}
