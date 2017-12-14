SELECT *
FROM
	(SELECT O.customerNumber custNum, OD.productCode prodCode
	 FROM Orders O, orderDetails OD
	 WHERE O.orderNumber = OD.orderNumber AND O.customerNumber != '219' AND prodCode IN
		(SELECT OD.productCode prodCode
		FROM Orders O, orderDetails OD
		WHERE O.orderNumber = OD.orderNumber AND O.customerNumber = '219'
		ORDER BY O.customerNumber)
	GROUP BY custNum HAVING COUNT(DISTINCT prodCode) >= 3);