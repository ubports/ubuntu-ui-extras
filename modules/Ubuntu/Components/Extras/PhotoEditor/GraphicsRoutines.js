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

/* Given 'input', constrain the value of 'input' to range between
 * 'lowConstraint' and 'highConstraint', inclusive. Return the constrained
 * value without modifying 'input'.
 */
function clamp(input, lowConstraint, highConstraint) {
  if (input < lowConstraint)
    return lowConstraint;
  else if (input > highConstraint)
    return highConstraint;
  else
    return input;
}

function cloneRect(source) {
  var ret = { };
  ret.x = source.x;
  ret.y = source.y;
  ret.width = source.width;
  ret.height = source.height;
  
  return ret;
}

function interpolateRect(start, end, factor) {
  var result = { };

  result.x = start.x + factor * (end.x - start.x);
  result.y = start.y + factor * (end.y - start.y);
  result.width = start.width + factor * (end.width - start.width);
  result.height = start.height + factor * (end.height - start.height);

  return result;
}

/* Forces Geometry object 'item' to fit centered inside Geometry object
 * 'viewport', preserving the aspect of ratio of 'item' but potentially scaling
 * and translating it so that it snugly fits centered inside of 'viewport'.
 * Return the new scaled-up and translated Geometry for 'item'.
 */
function fitRect(viewport, item) {
  if (item.width == 0 || item.height == 0) {
      return viewport;
  }

  var itemAspectRatio = item.width / item.height;
  var viewportAspectRatio = viewport.width / viewport.height;

  var result = { };
  if (itemAspectRatio > viewportAspectRatio) {
    var scaleFactor = viewport.width / item.width;
    result.width = viewport.width;
    result.height = item.height * scaleFactor
  } else {
    scaleFactor = viewport.height / item.height;
    result.width = item.width * scaleFactor
    result.height = viewport.height;
  }

  result.width = clamp(result.width, 0, viewport.width);
  result.height = clamp(result.height, 0, viewport.height);
  result.x = viewport.x + (viewport.width - result.width) / 2;
  result.y = viewport.y + (viewport.height - result.height) / 2;
  result.scaleFactor = scaleFactor;

  return result;
}

function getRelativeRect(geom, relativeTo) {
  var result = { };

  result.x = (geom.x - relativeTo.x) / relativeTo.width;
  result.y = (geom.y - relativeTo.y) / relativeTo.height;
  result.width = geom.width / relativeTo.width;
  result.height = geom.height / relativeTo.height;

  return result;
}

function sizeToRect(rect, qmlItem) {
  qmlItem.x = rect.x;
  qmlItem.y = rect.y;
  qmlItem.width = rect.width;
  qmlItem.height = rect.height;
}

function areEqual(geom1, geom2) {
  return (geom1.x === geom2.x && geom1.y === geom2.y && geom1.width ===
          geom2.width && geom1.height === geom2.height);
}
