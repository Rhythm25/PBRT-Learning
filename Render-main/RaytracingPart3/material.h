#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"
//#include "hittable.h"
#include "texture.h"
#include "onb.h"
#include "pdf.h"
#include "spectrum.h"
#include "primitive.h"

#ifdef PBRT_SAMPLED_SPECTRUM
typedef SampledSpectrum Spectrum;
#else
typedef RGBSpectrum Spectrum;
#endif

//struct SurfaceInteraction;

struct scatter_record {
    ray specular_ray;
    bool is_specular;
    color attenuation;
    Spectrum attenuationSpe;
    shared_ptr<pdf> pdf_ptr;
};

class material {
public:
    virtual bool scatter(
        const ray& r_in, const SurfaceInteraction& rec, scatter_record& srec
    ) const {
        return false;
    }

    virtual double scattering_pdf(
        const ray& r_in, const SurfaceInteraction& rec, const ray& scattered
    ) const {
        return 0;
    }
    virtual color emitted(const ray& r_in, const SurfaceInteraction& rec, double u, double v, const Point3f& p) const {
        return color(0, 0, 0);
    }
};

class lambertian : public material {
public:
    lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}
    lambertian(const Spectrum& spe) : albedo(make_shared<solid_color>(spe.ToColor())) {}

    virtual bool scatter(
        const ray& r_in, const SurfaceInteraction& rec, scatter_record& srec
    ) const override {
        srec.is_specular = false;
        srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
        srec.attenuationSpe = Spectrum::FromRGB(albedo->value(rec.u, rec.v, rec.p).ToArray());
        srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
        return true;
    }
    double scattering_pdf(
        const ray& r_in, const SurfaceInteraction& rec, const ray& scattered
    ) const {
        auto cosine = Dot(rec.normal, Normalize(scattered.direction()));
        return cosine < 0 ? 0 : cosine / PI;
    }


public:
   // color albedo;
    shared_ptr<texture> albedo;
};

class metal : public material {
public:
    metal(const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}
    metal(const Spectrum& spe, double f) : albedo(spe.ToColor()), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(
        const ray& r_in, const SurfaceInteraction& rec, scatter_record& srec
    ) const override {
        Vector3f reflected = reflect(Normalize(r_in.direction()), rec.normal);
        srec.specular_ray = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        srec.attenuation = albedo;
        double rgb[3] = { albedo.x,albedo.y,albedo.z };
        srec.attenuationSpe = Spectrum::FromRGB(rgb);
        srec.is_specular = true;
        srec.pdf_ptr = 0;
        return true;
    }

public:
    color albedo;
    double fuzz;
};

class dielectric : public material {
public:
    dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(
        const ray& r_in, const SurfaceInteraction& rec, scatter_record& srec
    ) const override {
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        srec.attenuation = color(1.0, 1.0, 1.0);
        srec.attenuationSpe = Spectrum::FromRGB(color(1.0, 1.0, 1.0).ToArray());
        double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

        Vector3f unit_direction = Normalize(r_in.direction());
        double cos_theta = fmin(Dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        Vector3f direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, refraction_ratio);

        srec.specular_ray = ray(rec.p, direction, r_in.time());
        return true;
    }

public:
    double ir; // Index of Refraction
private:
    static double reflectance(double cosine, double ref_idx) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

class diffuse_light : public material {
public:
    diffuse_light(shared_ptr<texture> a) : emit_light(a) {}
    diffuse_light(color c) : emit_light(make_shared<solid_color>(c)) {}
    diffuse_light(const Spectrum& spe) : emit_light(make_shared<solid_color>(spe.ToColor())) {}

    virtual bool scatter(
        const ray& r_in, const SurfaceInteraction& rec, scatter_record& srec
    ) const override {
        return false;
    }

    virtual color emitted(const ray& r_in, const SurfaceInteraction& rec, double u, double v,
        const Point3f& p) const override {

        if (rec.front_face) {
            //cout << "1" << endl;
            return emit_light->value(u, v, p);}
       
        else {
            //return emit_light->value(u, v, p);
            return color(0, 0, 0);}

            
    }

public:
    shared_ptr<texture> emit_light;
};

/*class isotropic : public material {
public:
    isotropic(color c) : albedo(make_shared<solid_color>(c)) {}
    isotropic(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        scattered = ray(rec.p, random_in_unit_sphere(), r_in.time());
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }

public:
    shared_ptr<texture> albedo;
};*/

#endif