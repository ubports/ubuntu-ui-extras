/*
 * Copyright (C) 2012 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 * Lucas Beeler <lucas@yorba.org>
 */


#include <QApplication>
#include <qmath.h>

#include "imaging.h"

/*!
 * \brief HSVTransformation::transformPixel
 * \param pixel_color
 * \return
 */
QColor HSVTransformation::transformPixel(const QColor &pixel_color) const
{
    QColor result;

    int h, s, v;
    pixel_color.getHsv(&h, &s, &v);

    v = remap_table_[v];

    result.setHsv(h, s, v);

    return result;
}

/*!
 * \brief IntensityHistogram::IntensityHistogram
 * \param basis_image
 */
IntensityHistogram::IntensityHistogram(const QImage& basis_image)
{
    for (int i = 0; i < 256; i++)
        m_counts[i] = 0;

    int width = basis_image.width();
    int height = basis_image.height();

    for (int j = 0; j < height; j++) {
        QApplication::processEvents();

        for (int i = 0; i < width; i++) {
            QColor c = QColor(basis_image.pixel(i, j));
            int intensity = c.value();
            m_counts[intensity]++;
        }
    }

    float pixel_count = (float)(width * height);
    float accumulator = 0.0f;
    for (int i = 0; i < 256; i++) {
        m_probabilities[i] = ((float) m_counts[i]) / pixel_count;
        accumulator += m_probabilities[i];
        m_cumulativeProbabilities[i] = accumulator;
    }
}

/*!
 * \brief IntensityHistogram::getCumulativeProbability
 * \param level
 * \return
 */
float IntensityHistogram::getCumulativeProbability(int level)
{
    return m_cumulativeProbabilities[level];
}


const float ToneExpansionTransformation::DEFAULT_LOW_DISCARD_MASS = 0.02f;
const float ToneExpansionTransformation::DEFAULT_HIGH_DISCARD_MASS = 0.98f;
/*!
 * \brief ToneExpansionTransformation::ToneExpansionTransformation
 * \param h
 * \param low_discard_mass
 * \param high_discard_mass
 */
ToneExpansionTransformation::ToneExpansionTransformation(IntensityHistogram h,
                                                         float low_discard_mass, float high_discard_mass)
{
    if (low_discard_mass == -1.0f)
        low_discard_mass = DEFAULT_LOW_DISCARD_MASS;
    if (high_discard_mass == -1.0f)
        high_discard_mass = DEFAULT_HIGH_DISCARD_MASS;

    m_lowDiscardMass = low_discard_mass;
    m_highDiscardMass = high_discard_mass;

    m_lowKink = 0;
    m_highKink = 255;

    while (h.getCumulativeProbability(m_lowKink) < low_discard_mass)
        m_lowKink++;

    while (h.getCumulativeProbability(m_highKink) > high_discard_mass)
        m_highKink--;

    m_lowKink = clampi(m_lowKink, 0, 255);
    m_highKink = clampi(m_highKink, 0, 255);

    buildRemapTable();
}

/*!
 * \brief ToneExpansionTransformation::isIdentity
 * \return
 */
bool ToneExpansionTransformation::isIdentity() const
{
    return ((m_lowKink == 0) && (m_highKink == 255));
}

/*!
 * \brief ToneExpansionTransformation::buildRemapTable
 */
void ToneExpansionTransformation::buildRemapTable()
{
    float low_kink_f = ((float) m_lowKink) / 255.0f;
    float high_kink_f = ((float) m_highKink) / 255.0f;

    float slope = 1.0f / (high_kink_f - low_kink_f);
    float intercept = -(low_kink_f / (high_kink_f - low_kink_f));

    int i = 0;
    for ( ; i <= m_lowKink; i++)
        remap_table_[i] = 0;

    for ( ; i < m_highKink; i++)
        remap_table_[i] = (int) ((255.0f * (slope * (((float) i) / 255.0f) +
                                            intercept)) + 0.5);

    for ( ; i < 256; i++)
        remap_table_[i] = 255;
}

/*!
 * \brief ToneExpansionTransformation::lowDiscardMass
 * \return
 */
float ToneExpansionTransformation::lowDiscardMass() const
{
    return m_lowDiscardMass;
}

/*!
 * \brief ToneExpansionTransformation::highDiscardMass
 * \return
 */
float ToneExpansionTransformation::highDiscardMass() const
{
    return m_highDiscardMass;
}


/*!
 * \brief HermiteGammaApproximationFunction::HermiteGammaApproximationFunction
 * \param user_interval_upper
 */
HermiteGammaApproximationFunction::HermiteGammaApproximationFunction(
        float user_interval_upper)
{
    m_nonzeroIntervalUpper = clampf(user_interval_upper, 0.1f, 1.0f);
    m_xScale = 1.0f / m_nonzeroIntervalUpper;
}

/*!
 * \brief HermiteGammaApproximationFunction::evaluate
 * \param x
 * \return
 */
float HermiteGammaApproximationFunction::evaluate(float x)
{
    if (x < 0.0f)
        return 0.0f;
    else if (x > m_nonzeroIntervalUpper)
        return 0.0f;
    else {
        float indep_var = m_xScale * x;

        float dep_var =  6.0f * ((indep_var * indep_var * indep_var) -
                                 (2.0f * (indep_var * indep_var)) + (indep_var));

        return clampf(dep_var, 0.0f, 1.0f);
    }
}


const float ShadowDetailTransformation::MAX_EFFECT_SHIFT = 0.5f;
const float ShadowDetailTransformation::MIN_TONAL_WIDTH = 0.1f;
const float ShadowDetailTransformation::MAX_TONAL_WIDTH = 1.0f;
const float ShadowDetailTransformation::TONAL_WIDTH = 1.0f;
/*!
 * \brief ShadowDetailTransformation::ShadowDetailTransformation
 * \param intensity
 */
ShadowDetailTransformation::ShadowDetailTransformation(float intensity)
{
    m_intensity = intensity;
    float effect_shift = MAX_EFFECT_SHIFT * intensity;

    HermiteGammaApproximationFunction func =
            HermiteGammaApproximationFunction(TONAL_WIDTH);

    for (int i = 0; i < 256; i++) {
        float x = ((float) i) / 255.0f;
        float weight = func.evaluate(x);

        int remapped = (int) ((255.0f * (weight * (x + effect_shift)) + ((1.0f -
                                                                          weight) * x)) + 0.5f);
        remap_table_[i] = clampi(remapped, i, 255);
    }
}

/*!
 * \brief ShadowDetailTransformation::isIdentity
 * \return
 */
bool ShadowDetailTransformation::isIdentity() const
{
    return (m_intensity == 0.0f);
}


const int AutoEnhanceTransformation::SHADOW_DETECT_MIN_INTENSITY = 2;
const int AutoEnhanceTransformation::SHADOW_DETECT_MAX_INTENSITY = 90;
const int AutoEnhanceTransformation::SHADOW_DETECT_INTENSITY_RANGE =
        AutoEnhanceTransformation::SHADOW_DETECT_MAX_INTENSITY -
        AutoEnhanceTransformation::SHADOW_DETECT_MIN_INTENSITY;
const int AutoEnhanceTransformation::EMPIRICAL_DARK = 40;
const float AutoEnhanceTransformation::SHADOW_AGGRESSIVENESS_MUL = 0.45f;
/*!
 * \brief AutoEnhanceTransformation::AutoEnhanceTransformation
 * \param basis
 */
AutoEnhanceTransformation::AutoEnhanceTransformation(const QImage& basis)
    : m_shadowTransform(0), m_toneExpansionTransform(0)
{
    IntensityHistogram histogram = IntensityHistogram(basis);

    /* compute the percentage of pixels in the image that fall into the
     shadow range -- this measures "of the pixels in the image, how many of
     them are in shadow?" */
    float pct_in_range = 100.0f *
            (histogram.getCumulativeProbability(SHADOW_DETECT_MAX_INTENSITY) -
             histogram.getCumulativeProbability(SHADOW_DETECT_MIN_INTENSITY));

    /* compute the mean intensity of the pixels that are in the shadow range --
     this measures "of those pixels that are in shadow, just how dark are
     they?" */
    float sh_prob_mu =
            (histogram.getCumulativeProbability(SHADOW_DETECT_MIN_INTENSITY) +
             histogram.getCumulativeProbability(SHADOW_DETECT_MAX_INTENSITY)) * 0.5f;
    int sh_intensity_mu = SHADOW_DETECT_MIN_INTENSITY;
    for ( ; sh_intensity_mu <= SHADOW_DETECT_MAX_INTENSITY; sh_intensity_mu++) {
        if (histogram.getCumulativeProbability(sh_intensity_mu) >= sh_prob_mu)
            break;
    }

    /* if more than 30 percent of the pixels in the image are in the shadow
     detection range, or if the mean intensity within the shadow range is less
     than an empirically determined threshold below which pixels appear very
     dark, regardless of the percent of pixels in it, then perform shadow
     detail enhancement. Otherwise, skip shadow detail enhancement and perform
     contrast expansion only */
    if ((pct_in_range > 30.0f) || ((pct_in_range > 10.0f) &&
                                   (sh_intensity_mu < EMPIRICAL_DARK))) {
        float shadow_trans_effect_size = ((((float) SHADOW_DETECT_MAX_INTENSITY) -
                                           ((float) sh_intensity_mu)) /
                                          ((float) SHADOW_DETECT_INTENSITY_RANGE));
        shadow_trans_effect_size *= SHADOW_AGGRESSIVENESS_MUL;

        m_shadowTransform
                = new ShadowDetailTransformation(shadow_trans_effect_size);

        QImage shadow_corrected_image = QImage(basis);
        // Can't write into indexed images, due to a limitation in Qt.
        if (shadow_corrected_image.format() == QImage::Format_Indexed8)
            shadow_corrected_image = shadow_corrected_image.convertToFormat(
                        QImage::Format_RGB32);

        for (int j = 0; j < shadow_corrected_image.height(); j++) {
            QApplication::processEvents();

            for (int i = 0; i < shadow_corrected_image.width(); i++) {
                QColor px = m_shadowTransform->transformPixel(
                            QColor(shadow_corrected_image.pixel(i, j)));
                shadow_corrected_image.setPixel(i, j, px.rgb());
            }
        }

        m_toneExpansionTransform = new ToneExpansionTransformation(
                    IntensityHistogram(shadow_corrected_image), 0.005f, 0.995f);

    } else {
        m_toneExpansionTransform = new ToneExpansionTransformation(
                    IntensityHistogram(basis));
    }
}

/*!
 * \brief AutoEnhanceTransformation::~AutoEnhanceTransformation
 */
AutoEnhanceTransformation::~AutoEnhanceTransformation()
{
    if (m_shadowTransform)
        delete m_shadowTransform;
    delete m_toneExpansionTransform;
}

/*!
 * \brief AutoEnhanceTransformation::transformPixel
 * \param pixel_color
 * \return
 */
QColor AutoEnhanceTransformation::transformPixel(
        const QColor& pixel_color) const
{
    QColor px = pixel_color;

    if (m_shadowTransform)
        px = m_shadowTransform->transformPixel(px);

    px = m_toneExpansionTransform->transformPixel(px);

    /* if tone expansion occurs, boost saturation to compensate for boosted
     dynamic range */
    if (!m_toneExpansionTransform->isIdentity()) {
        int h, s, v;
        px.getHsv(&h, &s, &v);

        float compensation_multiplier =
                (m_toneExpansionTransform->lowDiscardMass() < 0.01f) ? 1.02f : 1.10f;

        s = (int) (((float) s) * compensation_multiplier);
        s = clampi(s, 0, 255);

        px.setHsv(h, s, v);
    }

    return px;
}

bool AutoEnhanceTransformation::isIdentity() const
{
    return false;
}


/*!
 * \brief ColorBalance::ColorBalance
 * \param brightness 0 is total dark, 1 is as the original, grater than 1 is brigther
 * \param contrast from 0 maybe 5. 1 is as the original
 * \param saturation from 0 maybe 5. 1 is as the original
 * \param hue from 0 to 360. 0 and 360 is as the original
 */
ColorBalance::ColorBalance(qreal brightness, qreal contrast, qreal saturation, qreal hue)
{
    qreal cos_h = qCos(hue * (M_PI / 180.0));
    qreal sin_h = qSin(hue * (M_PI / 180.0));
    h1 = QVector4D(0.333333 * (1.0 - cos_h) + cos_h,
                   0.333333 * (1.0 - cos_h) + 0.57735 * sin_h,
                   0.333333 * (1.0 - cos_h) - 0.57735 * sin_h,
                   0.0);
    h2 = QVector4D(0.333333 * (1.0 - cos_h) - 0.57735 * sin_h,
                   0.333333 * (1.0 - cos_h) + cos_h,
                   0.333333 * (1.0 - cos_h) + 0.57735 * sin_h,
                   0.0);
    h3 = QVector4D(0.333333 * (1.0 - cos_h) + 0.57735 * sin_h,
                   0.333333 * (1.0 - cos_h) - 0.57735 * sin_h,
                   0.333333 * (1.0 - cos_h) + cos_h,
                   0.0);

    s1 = QVector4D((1.0 - saturation) * 0.3086 + saturation,
                   (1.0 - saturation) * 0.6094,
                   (1.0 - saturation) * 0.0820,
                   0.0);
    s2 = QVector4D((1.0 - saturation) * 0.3086,
                   (1.0 - saturation) * 0.6094 + saturation,
                   (1.0 - saturation) * 0.0820,
                   0.0);
    s3 = QVector4D((1.0 - saturation) * 0.3086,
                   (1.0 - saturation) * 0.6094,
                   (1.0 - saturation) * 0.0820 + saturation,
                   0.0);

    b1 = QVector4D(brightness, 0.0, 0.0, 0.0);
    b2 = QVector4D(0.0, brightness, 0.0, 0.0);
    b3 = QVector4D(0.0, 0.0, brightness, 0.0);

    c1 = QVector4D(contrast, 0.0, 0.0, contrast * -0.5 + 0.5);
    c2 = QVector4D(0.0, contrast, 0.0, contrast * -0.5 + 0.5);
    c3 = QVector4D(0.0, 0.0, contrast, contrast * -0.5 + 0.5);
}

/*!
 * \brief ColorBalance::transformPixel transforms one pixel according to the parameters given in
 * the constructor
 * \param pixel_color The pixel to be transformed
 * \return Color for the new pixel
 */
QColor ColorBalance::transformPixel(const QColor &pixel_color) const
{
    QVector4D pixel(pixel_color.red()/255.0, pixel_color.green()/255.0, pixel_color.blue()/255.0, 0.0);
    pixel = transformHue(pixel);
    pixel = transformSaturation(pixel);
    pixel = transformBrightness(pixel);
    pixel = transformContrast(pixel);
    int red = qBound(0, (int)(pixel.x()*255), 255);
    int green = qBound(0, (int)(pixel.y()*255), 255);
    int blue = qBound(0, (int)(pixel.z()*255), 255);
    return QColor(red, green, blue);
}

/*!
 * \brief ColorBalance::transformBrightness transform the brightness
 * \param pixel the color in a 4D vector with (R, G, B, 1.0)
 * \return the brightness hue transformed color again as 4D vector
 */
QVector4D ColorBalance::transformBrightness(const QVector4D &pixel) const
{
    return QVector4D(QVector4D::dotProduct(b1, pixel), QVector4D::dotProduct(b2, pixel),
                     QVector4D::dotProduct(b3, pixel), 1.0);
}

/*!
 * \brief ColorBalance::transformContrast transforms the Contrast
 * \param pixel the color in a 4D vector with (R, G, B, 1.0)
 * \return the new contrast transformed color again as 4D vector
 */
QVector4D ColorBalance::transformContrast(const QVector4D &pixel) const
{
    return QVector4D(QVector4D::dotProduct(c1, pixel), QVector4D::dotProduct(c2, pixel),
                     QVector4D::dotProduct(c3, pixel), 1.0);
}

/*!
 * \brief ColorBalance::transformSaturation transforms the saturation
 * \param pixel the color in a 4D vector with (R, G, B, 1.0)
 * \return the new saturation transformed color again as 4D vector
 */
QVector4D ColorBalance::transformSaturation(const QVector4D &pixel) const
{
    return QVector4D(QVector4D::dotProduct(s1, pixel), QVector4D::dotProduct(s2, pixel),
                     QVector4D::dotProduct(s3, pixel), 1.0);
}

/*!
 * \brief ColorBalance::transformHue transforms the hue
 * \param pixel the color in a 4D vector with (R, G, B, 1.0)
 * \return the new hue transformed color again as 4D vector
 */
QVector4D ColorBalance::transformHue(const QVector4D &pixel) const
{
    return QVector4D(QVector4D::dotProduct(h1, pixel), QVector4D::dotProduct(h2, pixel),
                     QVector4D::dotProduct(h3, pixel), 1.0);
}
