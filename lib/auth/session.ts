import { NextRequest } from "next/server";
import { verifyToken, TokenPayload } from "./jwt";

export function getSessionUser(req: NextRequest): TokenPayload | null {
  // Check Authorization Bearer header
  const authHeader = req.headers.get("authorization");
  if (authHeader && authHeader.startsWith("Bearer ")) {
    const token = authHeader.substring(7);
    const decoded = verifyToken(token);
    if (decoded) return decoded;
  }

  // Check Cookie
  const cookie = req.cookies.get("auth_token");
  if (cookie && cookie.value) {
    return verifyToken(cookie.value);
  }

  return null;
}

export function isAuthorized(
  user: TokenPayload | null,
  allowedRoles: Array<"ADMIN" | "FACULTY" | "STUDENT">
): boolean {
  if (!user) return false;
  return allowedRoles.includes(user.role);
}
