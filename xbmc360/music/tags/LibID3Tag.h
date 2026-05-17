#pragma once

/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

// Xbox 360 port: Static-link wrapper for libid3tag.
// In xbmc4xbox this was a DLL dynamic loader (DllDynamic).
// Since we statically link libid3tag on Xbox 360, this class
// simply forwards calls directly to the libid3tag C API.
// Follows the same CLib<Name> convention as CLibMad / CLibFlac.

#include "id3tag.h"
#include "metadata.h"

class CLibID3Tag
{
public:
	CLibID3Tag() : m_bLoaded(false) {}
	~CLibID3Tag() {}

	bool Load() { m_bLoaded = true; return true; }
	bool IsLoaded() { return m_bLoaded; }

	// file interface
	struct id3_file *id3_file_open(char const *path, enum id3_file_mode mode) { return ::id3_file_open(path, mode); }
	struct id3_file *id3_file_fdopen(int fd, enum id3_file_mode mode) { return ::id3_file_fdopen(fd, mode); }
	int id3_file_close(struct id3_file *file) { return ::id3_file_close(file); }
	struct id3_tag *id3_file_tag(struct id3_file const *file) { return ::id3_file_tag(file); }
	int id3_file_update(struct id3_file *file) { return ::id3_file_update(file); }

	// tag interface
	struct id3_tag *id3_tag_new(void) { return ::id3_tag_new(); }
	void id3_tag_delete(struct id3_tag *tag) { ::id3_tag_delete(tag); }
	unsigned int id3_tag_version(struct id3_tag const *tag) { return ::id3_tag_version(tag); }
	int id3_tag_options(struct id3_tag *tag, int mask, int values) { return ::id3_tag_options(tag, mask, values); }
	void id3_tag_setlength(struct id3_tag *tag, id3_length_t length) { ::id3_tag_setlength(tag, length); }
	void id3_tag_clearframes(struct id3_tag *tag) { ::id3_tag_clearframes(tag); }
	int id3_tag_attachframe(struct id3_tag *tag, struct id3_frame *frame) { return ::id3_tag_attachframe(tag, frame); }
	int id3_tag_detachframe(struct id3_tag *tag, struct id3_frame *frame) { return ::id3_tag_detachframe(tag, frame); }
	struct id3_frame *id3_tag_findframe(struct id3_tag const *tag, char const *id, unsigned int index) { return ::id3_tag_findframe(tag, id, index); }
	signed long id3_tag_query(id3_byte_t const *data, id3_length_t length) { return ::id3_tag_query(data, length); }
	struct id3_tag *id3_tag_parse(id3_byte_t const *data, id3_length_t length) { return ::id3_tag_parse(data, length); }
	id3_length_t id3_tag_render(struct id3_tag const *tag, id3_byte_t *buffer) { return ::id3_tag_render(tag, buffer); }

	// frame interface
	struct id3_frame *id3_frame_new(char const *id) { return ::id3_frame_new(id); }
	void id3_frame_delete(struct id3_frame *frame) { ::id3_frame_delete(frame); }
	union id3_field *id3_frame_field(struct id3_frame const *frame, unsigned int index) { return ::id3_frame_field(frame, index); }

	// field interface
	enum id3_field_type id3_field_type(union id3_field const *field) { return ::id3_field_type(field); }
	int id3_field_setint(union id3_field *field, signed long value) { return ::id3_field_setint(field, value); }
	int id3_field_settextencoding(union id3_field *field, enum id3_field_textencoding enc) { return ::id3_field_settextencoding(field, enc); }
	int id3_field_setstrings(union id3_field *field, unsigned int n, id3_ucs4_t **strings) { return ::id3_field_setstrings(field, n, strings); }
	int id3_field_addstring(union id3_field *field, id3_ucs4_t const *str) { return ::id3_field_addstring(field, str); }
	int id3_field_setlanguage(union id3_field *field, char const *lang) { return ::id3_field_setlanguage(field, lang); }
	int id3_field_setlatin1(union id3_field *field, id3_latin1_t const *str) { return ::id3_field_setlatin1(field, str); }
	int id3_field_setfulllatin1(union id3_field *field, id3_latin1_t const *str) { return ::id3_field_setfulllatin1(field, str); }
	int id3_field_setstring(union id3_field *field, id3_ucs4_t const *str) { return ::id3_field_setstring(field, str); }
	int id3_field_setfullstring(union id3_field *field, id3_ucs4_t const *str) { return ::id3_field_setfullstring(field, str); }
	int id3_field_setframeid(union id3_field *field, char const *id) { return ::id3_field_setframeid(field, id); }
	int id3_field_setbinarydata(union id3_field *field, id3_byte_t const *data, id3_length_t length) { return ::id3_field_setbinarydata(field, data, length); }
	signed long id3_field_getint(union id3_field const *field) { return ::id3_field_getint(field); }
	enum id3_field_textencoding id3_field_gettextencoding(union id3_field const *field) { return ::id3_field_gettextencoding(field); }
	id3_latin1_t const *id3_field_getlatin1(union id3_field const *field) { return ::id3_field_getlatin1(field); }
	id3_latin1_t const *id3_field_getfulllatin1(union id3_field const *field) { return ::id3_field_getfulllatin1(field); }
	id3_ucs4_t const *id3_field_getstring(union id3_field const *field) { return ::id3_field_getstring(field); }
	id3_ucs4_t const *id3_field_getfullstring(union id3_field const *field) { return ::id3_field_getfullstring(field); }
	unsigned int id3_field_getnstrings(union id3_field const *field) { return ::id3_field_getnstrings(field); }
	id3_ucs4_t const *id3_field_getstrings(union id3_field const *field, unsigned int index) { return ::id3_field_getstrings(field, index); }
	char const *id3_field_getframeid(union id3_field const *field) { return ::id3_field_getframeid(field); }
	id3_byte_t const *id3_field_getbinarydata(union id3_field const *field, id3_length_t *length) { return ::id3_field_getbinarydata(field, length); }

	// genre interface
	id3_ucs4_t const *id3_genre_index(unsigned int index) { return ::id3_genre_index(index); }
	id3_ucs4_t const *id3_genre_name(id3_ucs4_t const *str) { return ::id3_genre_name(str); }
	int id3_genre_number(id3_ucs4_t const *str) { return ::id3_genre_number(str); }

	// ucs4 interface
	id3_latin1_t *id3_ucs4_latin1duplicate(id3_ucs4_t const *ucs4) { return ::id3_ucs4_latin1duplicate(ucs4); }
	id3_utf16_t *id3_ucs4_utf16duplicate(id3_ucs4_t const *ucs4) { return ::id3_ucs4_utf16duplicate(ucs4); }
	id3_utf8_t *id3_ucs4_utf8duplicate(id3_ucs4_t const *ucs4) { return ::id3_ucs4_utf8duplicate(ucs4); }
	void id3_ucs4_putnumber(id3_ucs4_t *ucs4, unsigned long number) { ::id3_ucs4_putnumber(ucs4, number); }
	unsigned long id3_ucs4_getnumber(id3_ucs4_t const *ucs4) { return ::id3_ucs4_getnumber(ucs4); }
	void id3_ucs4_free(id3_ucs4_t *ucs4) { ::id3_ucs4_free(ucs4); }

	// latin1/utf16/utf8 interfaces
	id3_ucs4_t *id3_latin1_ucs4duplicate(id3_latin1_t const *latin1) { return ::id3_latin1_ucs4duplicate(latin1); }
	id3_ucs4_t *id3_utf16_ucs4duplicate(id3_utf16_t const *utf16) { return ::id3_utf16_ucs4duplicate(utf16); }
	id3_ucs4_t *id3_utf8_ucs4duplicate(id3_utf8_t const *utf8) { return ::id3_utf8_ucs4duplicate(utf8); }
	void id3_latin1_free(id3_latin1_t *latin1) { ::id3_latin1_free(latin1); }
	void id3_utf16_free(id3_utf16_t *utf16) { ::id3_utf16_free(utf16); }
	void id3_utf8_free(id3_utf8_t *utf8) { ::id3_utf8_free(utf8); }

	// metadata interface (xbmc4xbox extensions)
	void id3_ucs4_list_free(id3_ucs4_list_t *list) { ::id3_ucs4_list_free(list); }
	const id3_ucs4_t* id3_metadata_getartist(const struct id3_tag *tag, enum id3_field_textencoding *enc) { return ::id3_metadata_getartist(tag, enc); }
	const id3_ucs4_t* id3_metadata_getalbum(const struct id3_tag *tag, enum id3_field_textencoding *enc) { return ::id3_metadata_getalbum(tag, enc); }
	const id3_ucs4_t* id3_metadata_getalbumartist(const struct id3_tag *tag, enum id3_field_textencoding *enc) { return ::id3_metadata_getalbumartist(tag, enc); }
	const id3_ucs4_t* id3_metadata_gettitle(const struct id3_tag *tag, enum id3_field_textencoding *enc) { return ::id3_metadata_gettitle(tag, enc); }
	const id3_ucs4_t* id3_metadata_gettrack(const struct id3_tag *tag, enum id3_field_textencoding *enc) { return ::id3_metadata_gettrack(tag, enc); }
	const id3_ucs4_t* id3_metadata_getpartofset(const struct id3_tag *tag, enum id3_field_textencoding *enc) { return ::id3_metadata_getpartofset(tag, enc); }
	const id3_ucs4_t* id3_metadata_getyear(const struct id3_tag *tag, enum id3_field_textencoding *enc) { return ::id3_metadata_getyear(tag, enc); }
	const id3_ucs4_t* id3_metadata_getgenre(const struct id3_tag *tag, enum id3_field_textencoding *enc) { return ::id3_metadata_getgenre(tag, enc); }
	id3_ucs4_list_t* id3_metadata_getgenres(const struct id3_tag *tag, enum id3_field_textencoding *enc) { return ::id3_metadata_getgenres(tag, enc); }
	const id3_ucs4_t* id3_metadata_getcomment(const struct id3_tag *tag, enum id3_field_textencoding *enc) { return ::id3_metadata_getcomment(tag, enc); }
	const id3_ucs4_t* id3_metadata_getencodedby(const struct id3_tag *tag, enum id3_field_textencoding *enc) { return ::id3_metadata_getencodedby(tag, enc); }
	char id3_metadata_getrating(const struct id3_tag *tag) { return ::id3_metadata_getrating(tag); }
	const id3_ucs4_t* id3_metadata_getcompilation(const struct id3_tag *tag, enum id3_field_textencoding *enc) { return ::id3_metadata_getcompilation(tag, enc); }
	int id3_metadata_haspicture(const struct id3_tag *tag, enum id3_picture_type pictype) { return ::id3_metadata_haspicture(tag, pictype); }
	const id3_latin1_t* id3_metadata_getpicturemimetype(const struct id3_tag *tag, enum id3_picture_type pictype) { return ::id3_metadata_getpicturemimetype(tag, pictype); }
	id3_byte_t const *id3_metadata_getpicturedata(const struct id3_tag *tag, enum id3_picture_type pictype, id3_length_t *length) { return ::id3_metadata_getpicturedata(tag, pictype, length); }
	id3_byte_t const *id3_metadata_getuniquefileidentifier(const struct id3_tag *tag, const char *owner, id3_length_t *length) { return ::id3_metadata_getuniquefileidentifier(tag, owner, length); }
	const id3_ucs4_t* id3_metadata_getusertext(const struct id3_tag *tag, const char *description) { return ::id3_metadata_getusertext(tag, description); }
	int id3_metadata_getfirstnonstandardpictype(const struct id3_tag *tag, enum id3_picture_type *pictype) { return ::id3_metadata_getfirstnonstandardpictype(tag, pictype); }
	int id3_metadata_setartist(struct id3_tag *tag, id3_ucs4_t *value) { return ::id3_metadata_setartist(tag, value); }
	int id3_metadata_setalbum(struct id3_tag *tag, id3_ucs4_t *value) { return ::id3_metadata_setalbum(tag, value); }
	int id3_metadata_setalbumartist(struct id3_tag *tag, id3_ucs4_t *value) { return ::id3_metadata_setalbumartist(tag, value); }
	int id3_metadata_settitle(struct id3_tag *tag, id3_ucs4_t *value) { return ::id3_metadata_settitle(tag, value); }
	int id3_metadata_settrack(struct id3_tag *tag, id3_ucs4_t *value) { return ::id3_metadata_settrack(tag, value); }
	int id3_metadata_setpartofset(struct id3_tag *tag, id3_ucs4_t *value) { return ::id3_metadata_setpartofset(tag, value); }
	int id3_metadata_setyear(struct id3_tag *tag, id3_ucs4_t *value) { return ::id3_metadata_setyear(tag, value); }
	int id3_metadata_setgenre(struct id3_tag *tag, id3_ucs4_t *value) { return ::id3_metadata_setgenre(tag, value); }
	int id3_metadata_setencodedby(struct id3_tag *tag, id3_ucs4_t *value) { return ::id3_metadata_setencodedby(tag, value); }
	int id3_metadata_setcomment(struct id3_tag *tag, id3_ucs4_t *value) { return ::id3_metadata_setcomment(tag, value); }
	int id3_metadata_setrating(struct id3_tag *tag, char value) { return ::id3_metadata_setrating(tag, value); }
	int id3_metadata_setcompilation(struct id3_tag *tag, id3_ucs4_t *value) { return ::id3_metadata_setcompilation(tag, value); }

private:
	bool m_bLoaded;
};
