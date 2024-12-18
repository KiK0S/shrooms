#pragma once

#include "../components/dynamic_object.hpp"
#include "../components/text_object.hpp"
#include "../components/textured_object.hpp"
#include "../components/init_object.hpp"
#include "geometry_system.hpp"
#include "../../utils/file_system.hpp"
#include <memory>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include "freetype-gl/freetype-gl.h"
#include "../../utils/logger.hpp"
#include "../../declarations/logger_system.hpp"

namespace text {


texture_atlas_t *atlas = 0;
texture_font_t *font = 0;
std::unique_ptr<texture::IntTextureObject> text_texture;
std::map<std::string, geometry::GeometryObject*> geometries;

struct TextSystem: public init::UnInitializedObject {
	TextSystem(): init::UnInitializedObject(5) {}

	void init() {
		std::cerr << "init text system\n";
		atlas = texture_atlas_new( 512, 512, 4 );
		std::string filename = file::asset("Vera.ttf");
		char * text = "abcdefghijklmnopqrstuvwxyz 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:;.,!?";
		int font_idx = 0;
		std::cerr << filename << '\n';
		font = texture_font_new_from_file( atlas, 80, filename.c_str() );
		texture_font_load_glyphs( font, text );
		text_texture = std::make_unique<texture::IntTextureObject>(texture::create_texture(atlas->width, atlas->height, atlas->data, GL_RGBA, GL_NEAREST));
		for (auto text_obj : text::texts) {
			init(text_obj);
		}
	}

	void init(text::TextObject* t) {
		std::string text = t->get_text();
		auto geom = arena::create<TextGeometry>(text);
		glm::vec2 pen(0.0, font->height);
		float max_y = 0.0;
		for (int i = 0; i < text.size(); i++) {
			char c = text[i];
			texture_glyph_t *glyph = texture_font_get_glyph( font, &c );
			if( glyph != NULL ) {
				float kerning = 0.0f;
				if( i > 0)
				{
						kerning = texture_glyph_get_kerning( glyph, &text[i - 1] );
				}
				pen.x += kerning;
				int x0  = (int)( pen.x + glyph->offset_x );
				int y0  = (int)( pen.y );
				int x1  = (int)( x0 + glyph->width );
				int y1  = (int)( y0 + glyph->height );
				max_y = std::max(max_y, (float)y1);
				float s0 = glyph->s0;
				float t0 = glyph->t0;
				float s1 = glyph->s1;
				float t1 = glyph->t1;
				LOG_IF(logger::enable_text_system_logging, "Glyph '" << c << "' metrics:\n"
						  << "Position: (" << x0 << "," << y0 << ") -> (" << x1 << "," << y1 << ")\n"
						  << "UV: (" << s0 << "," << t0 << ") -> (" << s1 << "," << t1 << ")\n"
						  << "Width: " << glyph->width << " Height: " << glyph->height << "\n"
						  << "Offsets: " << glyph->offset_x << "," << glyph->offset_y);
				geom->pos.push_back({x0, y0});
				geom->uv.push_back({s0, t1});
				geom->pos.push_back({x1, y1});
				geom->uv.push_back({s1, t0});
				geom->pos.push_back({x0, y1});
				geom->uv.push_back({s0, t0});
				geom->pos.push_back({x0, y0});
				geom->uv.push_back({s0, t1});
				geom->pos.push_back({x1, y0});
				geom->uv.push_back({s1, t1});
				geom->pos.push_back({x1, y1});
				geom->uv.push_back({s1, t0});
				pen.x += glyph->advance_x;
			}
		}

		for (auto& v : geom->pos) {
			v.x *= 2.0 / pen.x;
			v.x -= 1.0;
			v.y *= 2.0 / max_y;
			v.y -= 1.0;
		}
		LOG_IF(logger::enable_text_system_logging, "add to frame new geom");
		render_system::add_to_frame(geom);
		geometries[text] = geom;
		t->get_entity()->add(geom);
	}
};

} // namespace text_system
