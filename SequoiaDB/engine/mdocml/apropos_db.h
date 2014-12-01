/*******************************************************************************

   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = apropos_db.h

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/1/2014  ly  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef APROPOS_H
#define APROPOS_H

enum	restype {
	RESTYPE_MAN, /* man(7) file */
	RESTYPE_MDOC, /* mdoc(7) file */
	RESTYPE_CAT /* pre-formatted file */
};

struct	res {
	enum restype	 type; /* input file type */
	char		*file; /* file in file-system */
	char		*cat; /* category (3p, 3, etc.) */
	char		*title; /* title (FOO, etc.) */
	char		*arch; /* arch (or empty string) */
	char		*desc; /* description (from Nd) */
	unsigned int	 rec; /* record in index */
	/*
	 * The index volume.  This indexes into the array of directories
	 * searched for manual page databases.
	 */
	unsigned int	 volume;
	/*
	 * The following fields are used internally.
	 *
	 * Maintain a binary tree for checking the uniqueness of `rec'
	 * when adding elements to the results array.
	 * Since the results array is dynamic, use offset in the array
	 * instead of a pointer to the structure.
	 */
	int		 lhs;
	int		 rhs;
	int		 matched; /* expression is true */
	int		*matches; /* partial truth evaluations */
};

struct	opts {
	const char	*arch; /* restrict to architecture */
	const char	*cat; /* restrict to manual section */
};

__BEGIN_DECLS

struct	expr;

int		 apropos_search(int, char **, const struct opts *,
			const struct expr *, size_t, 
			void *, size_t *, struct res **,
			void (*)(struct res *, size_t, void *));
struct	expr	*exprcomp(int, char *[], size_t *);
void		 exprfree(struct expr *);
void	 	 resfree(struct res *, size_t);
struct	expr	*termcomp(int, char *[], size_t *);

__END_DECLS

#endif /*!APROPOS_H*/
